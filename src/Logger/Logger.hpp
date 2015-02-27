#ifndef LOGGER_HEADER
#define LOGGER_HEADER
#include <chrono>
#include <ostream>

template<typename CharT>
class Logger
{
public:
	using StreamType = std::basic_ostream<CharT>;
private:
	StreamType *m_ostream;
	bool m_enable_log;
	std::chrono::high_resolution_clock::time_point m_time_start;
public:
	void enable_log(bool enable_log);
	bool enable_log();
	
	void stream(StreamType &ostream);
	StreamType &stream();

	template<typename OutputT>
	void log(OutputT out, bool ts=true);

	Logger(StreamType &stream);
};

#include <Logger/Logger.inl>
#endif