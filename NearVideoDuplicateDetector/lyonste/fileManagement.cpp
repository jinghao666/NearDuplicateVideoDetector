#include "fileManagement.h"

namespace lyonste
{
	namespace fileManagement
	{
		std::ostream & operator<<(std::ostream & stream, const FileInfo & info)
		{
			return stream << info.getFilePathStr();
		}
	}
}