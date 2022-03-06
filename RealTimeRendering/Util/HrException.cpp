#include "HrException.h"

RTR::HrException::HrException(HRESULT result, const char* what_desc, const char* what_cmd, const char* file, int line)
{
    // Translate HRESULT to text
    const char* hresultMessage = "<unknown>";
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, result, 0, (LPSTR)&hresultMessage, 0, nullptr);

    // Build error message
    std::stringstream ss;
    ss << "Exception occured!" << std::endl
        << hresultMessage << " [Error Code: 0x" << std::hex << std::setw(sizeof(HRESULT) / 4) << result << std::dec << "]" << std::endl << std::endl
        << "Action: " << (what_desc ? what_desc : "<unknown>") << std::endl
        << "Code: " << (what_cmd ? what_cmd : "<unknown>") << std::endl
        << "File: " << (file ? file : "<unknown>") << std::endl
        << "Line: " << line << std::endl;

    m_exceptionText = ss.str();
}

const char* RTR::HrException::what() const noexcept
{
    return m_exceptionText.c_str();
}