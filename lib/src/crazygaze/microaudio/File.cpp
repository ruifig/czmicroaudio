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

namespace cz
{

namespace io
{

File::File(::cz::Core *core) : ::cz::Object(core)
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
	if (m_isInsideWriteString) return ERR_OK; // avoid re-entrance

	// set flag to avoid reentrance in case this file is a log file, and an error happens while logging.
	m_isInsideWriteString = true;
	int ret = ERR_OK;
	if (strlen(str)>0) ret = WriteData(str, strlen(str));
	if (ret==ERR_OK){
		int16_t nl = 0x0A0D;
		ret = WriteData(&nl, sizeof nl);
	}
	m_isInsideWriteString = false;
	return ret;	
}


} // namespace io
} // namespace cz

