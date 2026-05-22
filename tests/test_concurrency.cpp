#include <iostream>
#include "zenith/std/concurrency.hpp"
using namespace zenith::stdlib;

int passed = 0, failed = 0;
#define TEST(name) void name()
#define RUN_TEST(name) do { std::cout << #name "... "; try { name(); std::cout << "OK\n"; passed++; } catch(const std::exception& e) { std::cout << "FAIL: " << e.what() << "\n"; failed++; } } while(0)
#define ASSERT(x) if(!(x)) throw std::runtime_error("Assertion: " #x)

TEST(t_result_ok) { auto r = make_ok(42); ASSERT(r.is_ok()); ASSERT(r.value() == 42); }
TEST(t_result_err) { auto r = make_error<int>("e"); ASSERT(r.is_error()); ASSERT(r.error() == "e"); }
TEST(t_option_some) { auto o = make_some(10); ASSERT(o.is_some()); ASSERT(o.value() == 10); }
TEST(t_option_none) { auto o = make_none<int>(); ASSERT(o.is_none()); ASSERT(o.value_or(99) == 99); }
TEST(t_promise) { Promise<int> p; auto f = p.get_future(); p.set_value(7); ASSERT(f.is_ready()); ASSERT(f.get() == 7); }
TEST(t_promise_void) { Promise<void> p; auto f = p.get_future(); p.set_value(); ASSERT(f.is_ready()); f.get(); }
TEST(t_channel) { Channel<int> ch; std::thread t([&]{ch.send(1);ch.send(2);}); ASSERT(ch.receive()==1); ASSERT(ch.receive()==2); t.join(); }
TEST(t_executor) { auto ex = create_executor(2); auto f = ex->submit([]{return 42;}); ASSERT(f.get()==42); ex->shutdown(); }
TEST(t_actor) { std::atomic<int> c{0}; Actor<int> a([&](const int& m, Actor<int>& self){c+=m; if(c>=6)self.stop();}); a.send(1);a.send(2);a.send(3); std::this_thread::sleep_for(std::chrono::milliseconds(20)); ASSERT(c==6); ASSERT(!a.is_running()); }

int main() {
    std::cout << "=== Concurrency Tests ===\n";
    RUN_TEST(t_result_ok); RUN_TEST(t_result_err); RUN_TEST(t_option_some); RUN_TEST(t_option_none);
    RUN_TEST(t_promise); RUN_TEST(t_promise_void); RUN_TEST(t_channel); RUN_TEST(t_executor); RUN_TEST(t_actor);
    std::cout << "=========================\nPassed: " << passed << ", Failed: " << failed << "\n";
    return failed > 0 ? 1 : 0;
}
