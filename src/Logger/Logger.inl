#ifndef LOGGER_IMPL
#define LOGGER_IMPL
#include <Logger/Logger.hpp>
#include <iomanip>

template<typename CharT>
void Logger<CharT>::enable_log(bool enable_log){m_enable_log=enable_log;}
template<typename CharT>
bool Logger<CharT>::enable_log(){return m_enable_log;}

template<typename CharT>
void Logger<CharT>::stream(StreamType &ostream){m_ostream=&ostream;}
template<typename CharT>
typename Logger<CharT>::StreamType &Logger<CharT>::stream(){return *m_ostream;}

template<typename CharT>
template<typename OutputT>
void Logger<CharT>::log(OutputT out, bool ts)
{
	if(ts) {
		(*m_ostream)<<static_cast<CharT>('[');
		(*m_ostream)<<std::setw(15);
		(*m_ostream)<<(std::chrono::high_resolution_clock::now()-m_time_start).count();
		(*m_ostream)<<static_cast<CharT>(']');
		(*m_ostream)<<static_cast<CharT>(' ');
	}
	(*m_ostream)<<out;
	m_ostream->flush();
}

template<typename CharT>
Logger<CharT>::Logger(StreamType &stream):
	m_ostream{&stream},
	m_enable_log{true},
	m_time_start{std::chrono::high_resolution_clock::now()}
{;}
#endif