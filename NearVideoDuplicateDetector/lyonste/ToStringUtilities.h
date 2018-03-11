#pragma once

#include <iostream>
#include <chrono>

namespace lyonste
{
#define ENUM_STR(varName,enumVal) template<> constexpr const char* const varName <enumVal> = #enumVal

	// trim from start (in place)
	inline void ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	// trim from end (in place)
	inline void rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	// trim from both ends (in place)
	inline void trim(std::string &s) {
		ltrim(s);
		rtrim(s);
	}

	// trim from start (copying)
	static inline std::string ltrim_copy(std::string s) {
		ltrim(s);
		return s;
	}

	// trim from end (copying)
	static inline std::string rtrim_copy(std::string s) {
		rtrim(s);
		return s;
	}

	// trim from both ends (copying)
	static inline std::string trim_copy(std::string s) {
		trim(s);
		return s;
	}


	struct DataSizeToString
	{
		const double numBytes;

		constexpr DataSizeToString(double numBytes) :
			numBytes(numBytes)
		{

		}

		constexpr DataSizeToString(size_t numBytes) :
			numBytes(static_cast<double>(numBytes))
		{

		}
		friend std::ostream& operator<<(std::ostream& stream, const DataSizeToString& dataSize);

	};

	inline std::ostream& operator<<(std::ostream& stream, const DataSizeToString& dataSize)
	{
		double numBytes = dataSize.numBytes;
		if (numBytes >= (1024ull * 1024ull * 1024ull * 1024ull))
		{
			stream << (numBytes / (1024ull * 1024ull * 1024ull * 1024ull)) << "TB";
		}
		else if (numBytes >= (1024ull * 1024ull * 1024ull))
		{
			stream << (numBytes / (1024ull * 1024ull * 1024ull)) << "GB";
		}
		else if (numBytes >= (1024ull * 1024ull))
		{
			stream << (numBytes / (1024ull * 1024ull)) << "MB";
		}
		else if (numBytes >= 1024ull)
		{
			stream << (numBytes / (1024ull)) << "KB";
		}
		else
		{
			stream << numBytes << "B";
		}
		return stream;

	}

	struct DurationToString
	{

		const size_t millis;



		DurationToString(const std::chrono::system_clock::duration& dur) :
			millis(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count())
		{
		}

		DurationToString(double millis) :
			millis(size_t(millis))
		{
		}
		DurationToString(size_t total, size_t consumed, double numPerSec) :
			millis(size_t(double(total - consumed) / (numPerSec / 1000.0)))
		{
		}




		friend std::ostream& operator<<(std::ostream& stream, const DurationToString& durr);
	};

	inline std::ostream& operator<<(std::ostream& stream, const DurationToString& durr)
	{
		size_t millis = durr.millis;
		size_t hrs = millis / (1000 * 60 * 60);
		size_t min = (millis -= (hrs * 1000 * 60 * 60)) / (1000 * 60);
		size_t sec = (millis -= (min * 1000 * 60)) / 1000;
		millis -= (sec * 1000);
		stream << hrs;
		if (min>9)
		{
			stream << ":" << min;
		}
		else
		{
			stream << ":0" << min;
		}
		if (sec>9)
		{
			stream << ":" << sec;
		}
		else
		{
			stream << ":0" << sec;
		}
		if (millis>99)
		{
			stream << "." << millis;
		}
		else if (millis>9)
		{
			stream << ".0" << millis;
		}
		else
		{
			stream << ".00" << millis;
		}
		return stream;
	}
}