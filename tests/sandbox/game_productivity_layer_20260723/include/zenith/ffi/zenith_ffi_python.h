#ifndef ZENITH_FFI_PYTHON_H
#define ZENITH_FFI_PYTHON_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <vector>

// Python C API headers
#include <Python.h>

namespace zenith {
namespace ffi {

/**
 * PythonFFIBridge - Cross-platform Python FFI bridge
 * 
 * Provides embedded Python runtime integration for calling Python functions
 * from C++ with automatic type serialization and result conversion.
 */
class PythonFFIBridge {
public:
    /**
     * Initialize the Python runtime (call once at application startup)
     * @param python_home Optional path to Python installation
     */
    static void initialize(const std::string& python_home = "") {
        std::lock_guard<std::mutex> lock(init_mutex_);
        
        if (initialized_) {
            return; // Already initialized
        }
        
        if (!python_home.empty()) {
#ifdef _WIN32
            std::wstring wide_home(python_home.begin(), python_home.end());
            Py_SetPythonHome(const_cast<wchar_t*>(wide_home.c_str()));
#else
            std::wstring wide_home(python_home.begin(), python_home.end());
            Py_SetPythonHome(wide_home.c_str());
#endif
        }
        
        Py_Initialize();
        
        // Ensure thread support (PyEval_InitThreads is deprecated in Python 3.9+, no longer needed)
#if PY_VERSION_HEX < 0x03090000
        PyEval_InitThreads();
#endif
        
        initialized_ = true;
    }
    
    /**
     * Finalize the Python runtime (call once at application shutdown)
     */
    static void finalize() {
        std::lock_guard<std::mutex> lock(init_mutex_);
        
        if (initialized_) {
            Py_Finalize();
            initialized_ = false;
        }
    }
    
    /**
     * Check if Python runtime is initialized
     */
    static bool isInitialized() {
        return initialized_;
    }
    
    /**
     * Call a Python function from a module
     * @param module_name Name of the Python module
     * @param function_name Name of the function to call
     * @param args Arguments to pass to the function
     * @return String representation of the result
     * @throws std::runtime_error if call fails
     */
    template<typename... Args>
    static std::string callFunction(const std::string& module_name, 
                                    const std::string& function_name,
                                    Args&&... args) {
        if (!initialized_) {
            throw std::runtime_error("Python runtime not initialized. Call initialize() first.");
        }
        
        // Acquire GIL
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            // Import module
            PyObject* py_module = PyImport_ImportModule(module_name.c_str());
            if (!py_module) {
                std::string error = getPythonError();
                PyGILState_Release(gstate);
                throw std::runtime_error("Failed to import module '" + module_name + "': " + error);
            }
            
            // Get function
            PyObject* py_func = PyObject_GetAttrString(py_module, function_name.c_str());
            Py_DECREF(py_module);
            
            if (!py_func || !PyCallable_Check(py_func)) {
                std::string error = getPythonError();
                if (py_func) Py_DECREF(py_func);
                PyGILState_Release(gstate);
                throw std::runtime_error("Function '" + function_name + "' not found in module '" + module_name + "': " + error);
            }
            
            // Build arguments tuple
            PyObject* py_args = buildArguments(std::forward<Args>(args)...);
            
            // Call function
            PyObject* py_result = PyObject_CallObject(py_func, py_args);
            Py_DECREF(py_func);
            Py_DECREF(py_args);
            
            if (!py_result) {
                std::string error = getPythonError();
                PyGILState_Release(gstate);
                throw std::runtime_error("Python function call failed: " + error);
            }
            
            // Convert result to string
            std::string result = pythonToString(py_result);
            Py_DECREF(py_result);
            
            PyGILState_Release(gstate);
            return result;
            
        } catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    
    /**
     * Call a Python function and return as integer
     */
    template<typename... Args>
    static int callInt(const std::string& module_name, 
                       const std::string& function_name,
                       Args&&... args) {
        std::string result = callFunction(module_name, function_name, std::forward<Args>(args)...);
        try {
            return std::stoi(result);
        } catch (...) {
            throw std::runtime_error("Failed to convert Python result to int: " + result);
        }
    }
    
    /**
     * Call a Python function and return as double
     */
    template<typename... Args>
    static double callDouble(const std::string& module_name, 
                             const std::string& function_name,
                             Args&&... args) {
        std::string result = callFunction(module_name, function_name, std::forward<Args>(args)...);
        try {
            return std::stod(result);
        } catch (...) {
            throw std::runtime_error("Failed to convert Python result to double: " + result);
        }
    }
    
    /**
     * Call a Python function and return as boolean
     */
    template<typename... Args>
    static bool callBool(const std::string& module_name, 
                         const std::string& function_name,
                         Args&&... args) {
        std::string result = callFunction(module_name, function_name, std::forward<Args>(args)...);
        return (result == "true" || result == "1" || result == "True");
    }
    
    /**
     * Call a Python function and return as string
     */
    template<typename... Args>
    static std::string callString(const std::string& module_name, 
                                  const std::string& function_name,
                                  Args&&... args) {
        return callFunction(module_name, function_name, std::forward<Args>(args)...);
    }

private:
    static bool initialized_;
    static std::mutex init_mutex_;
    
    /**
     * Get Python error message
     */
    static std::string getPythonError() {
        PyObject *exc_type, *exc_value, *exc_traceback;
        PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
        
        std::string error = "Unknown error";
        if (exc_value) {
            PyObject* str_obj = PyObject_Str(exc_value);
            if (str_obj) {
                Py_ssize_t size;
                const char* c_str = PyUnicode_AsUTF8AndSize(str_obj, &size);
                if (c_str) {
                    error = std::string(c_str, size);
                }
                Py_DECREF(str_obj);
            }
        }
        
        PyErr_Clear();
        
        if (exc_type) Py_DECREF(exc_type);
        if (exc_value) Py_DECREF(exc_value);
        if (exc_traceback) Py_DECREF(exc_traceback);
        
        return error;
    }
    
    /**
     * Convert Python object to C++ string
     */
    static std::string pythonToString(PyObject* obj) {
        if (!obj) {
            return "";
        }
        
        if (PyUnicode_Check(obj)) {
            Py_ssize_t size;
            const char* c_str = PyUnicode_AsUTF8AndSize(obj, &size);
            if (c_str) {
                return std::string(c_str, size);
            }
        } else if (PyBool_Check(obj)) {
            return PyObject_IsTrue(obj) ? "true" : "false";
        } else if (PyLong_Check(obj)) {
            long val = PyLong_AsLong(obj);
            return std::to_string(val);
        } else if (PyFloat_Check(obj)) {
            double val = PyFloat_AsDouble(obj);
            std::ostringstream oss;
            oss << val;
            return oss.str();
        } else {
            // Generic conversion
            PyObject* str_obj = PyObject_Str(obj);
            if (str_obj) {
                Py_ssize_t size;
                const char* c_str = PyUnicode_AsUTF8AndSize(str_obj, &size);
                std::string result = c_str ? std::string(c_str, size) : "";
                Py_DECREF(str_obj);
                return result;
            }
        }
        
        return "";
    }
    
    /**
     * Build Python arguments tuple from variadic arguments
     */
    static PyObject* buildArguments() {
        return PyTuple_New(0);
    }
    
    // Overloads for different argument types
    static PyObject* buildArguments(int arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, PyLong_FromLong(arg));
        return tuple;
    }
    
    static PyObject* buildArguments(long arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, PyLong_FromLong(arg));
        return tuple;
    }
    
    static PyObject* buildArguments(double arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(arg));
        return tuple;
    }
    
    static PyObject* buildArguments(float arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(arg));
        return tuple;
    }
    
    static PyObject* buildArguments(bool arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, arg ? Py_True : Py_False);
        Py_INCREF(Py_True);  // or Py_False depending on arg
        return tuple;
    }
    
    static PyObject* buildArguments(const std::string& arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, PyUnicode_FromString(arg.c_str()));
        return tuple;
    }
    
    static PyObject* buildArguments(const char* arg) {
        PyObject* tuple = PyTuple_New(1);
        PyTuple_SetItem(tuple, 0, PyUnicode_FromString(arg));
        return tuple;
    }
    
    // Recursive variadic template for multiple arguments
    template<typename T, typename... Rest>
    static PyObject* buildArguments(T&& first, Rest&&... rest) {
        PyObject* first_tuple = buildArguments(std::forward<T>(first));
        PyObject* rest_tuple = buildArguments(std::forward<Rest>(rest)...);
        
        Py_ssize_t first_size = PyTuple_Size(first_tuple);
        Py_ssize_t rest_size = PyTuple_Size(rest_tuple);
        
        PyObject* combined = PyTuple_New(first_size + rest_size);
        
        for (Py_ssize_t i = 0; i < first_size; ++i) {
            PyObject* item = PyTuple_GetItem(first_tuple, i);
            Py_INCREF(item);
            PyTuple_SetItem(combined, i, item);
        }
        
        for (Py_ssize_t i = 0; i < rest_size; ++i) {
            PyObject* item = PyTuple_GetItem(rest_tuple, i);
            Py_INCREF(item);
            PyTuple_SetItem(combined, first_size + i, item);
        }
        
        Py_DECREF(first_tuple);
        Py_DECREF(rest_tuple);
        
        return combined;
    }
};

// Static member definitions
bool PythonFFIBridge::initialized_ = false;
std::mutex PythonFFIBridge::init_mutex_;

} // namespace ffi
} // namespace zenith

#endif // ZENITH_FFI_PYTHON_H
