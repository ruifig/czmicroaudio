//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/File.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>

namespace cz::microaudio
{

File::File()
{
	m_isOpen = false;
	m_swapBytes = false;
	m_isInsideWriteString = false;
}

File::~File(){
}


int File::GetSize(void)
{
	int save_pos = GetPos();
	Seek(0, FILE_SEEK_END);
	int len = GetPos();	
	Seek(save_pos, FILE_SEEK_START);
	return len;	
}


int File::WriteString(const char *str)
{
	if (m_isInsideWriteString) return Error::Success; // avoid re-entrance

	// set flag to avoid re entrance in case this file is a log file, and an error happens while logging.
	m_isInsideWriteString = true;
	int ret = Error::Success;
	if (strlen(str)>0) ret = WriteData(str, strlen(str));
	if (ret==Error::Success){
		int16_t nl = 0x0A0D;
		ret = WriteData(&nl, sizeof nl);
	}
	m_isInsideWriteString = false;
	return ret;	
}

} // namespace cz::microaudio

