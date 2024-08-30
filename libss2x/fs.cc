#include "fs.h"

namespace ss {

bool failure_services::sigint_handler_installed = false;
std::function<void(void)> failure_services::sigint_handler;
bool failure_services::sighup_handler_installed = false;
std::function<void(void)> failure_services::sighup_handler;

failure_services::failure_services()
: m_handler_installed(false)
, m_ignoring_signals(false)
{
	sigint_handler_installed = false;
	sighup_handler_installed = false;
}

failure_services::~failure_services()
{
	
}

failure_services& failure_services::get()
{
	static failure_services shared_instance;
	return shared_instance;
}

void failure_services::install_sigint_handler(std::function<void(void)> a_handler)
{
	sigint_handler = a_handler;
	sigint_handler_installed = true;
}

void failure_services::install_sighup_handler(std::function<void(void)> a_handler)
{
	sighup_handler = a_handler;
	sighup_handler_installed = true;
}

void failure_services::temporarily_ignore_signals()
{
	// ignore SIGINT and SIGHUP temporarily during a critical routine
	if (m_handler_installed) {
		m_ignoring_signals = true;
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
	}
}

void failure_services::unignore_signals()
{
	if (m_handler_installed) {
		signal(SIGINT, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		m_ignoring_signals = false;
	}
}

void failure_services::install_signal_handler()
{
	if (m_handler_installed)
		return;
		
	// handle signals of interest
	struct sigaction sa;
	sa.sa_handler = failure_services::handle_signal;
	sigemptyset(&sa.sa_mask);
	for (const auto& [ sig, name ] : signal_map) {
		sigaddset(&sa.sa_mask, sig);
	}
	sa.sa_flags = 0;
	for (const auto& [ sig, name] : signal_map) {
		if (sigaction(sig, &sa, NULL) < 0) {
			std::runtime_error e(std::format("fatal error: can't catch signal {} ({})", sig, name));
			throw (e);
		}
	}
	m_handler_installed = true;
}

void failure_services::invoke_sigint_handler()
{
	if (sigint_handler_installed)
		sigint_handler();
}

void failure_services::handle_signal(int signo)
{
	if ((signo == SIGINT) && sigint_handler_installed) {
		sigint_handler();
		return;
	}
	if ((signo == SIGHUP) && sighup_handler_installed) {
		sighup_handler();
		return;
	}
	std::cout << std::endl << std::endl;
	std::cout << "Caught signal " << signal_map.at(signo) << std::endl << std::endl;
	std::cout << "Stacktrace: " << std::endl << std::stacktrace::current() << std::endl;
	exit(EXIT_FAILURE);
}

}
