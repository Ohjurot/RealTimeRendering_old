#pragma once

#include <WinInclude.h>
#include <exception>
#include <sstream>
#include <string>
#include <iomanip>

#define RTR_HREXCEPTION(hr, what, lineOfCode) RTR::HrException(hr, what, lineOfCode, __FILE__, __LINE__)
#define RTR_CHECK_HRESULT(what, lineOfCode) {HRESULT __hr_temp; if(FAILED(__hr_temp = lineOfCode)) { throw RTR_HREXCEPTION(__hr_temp, what, #lineOfCode); }}

namespace RTR
{
    class HrException : public std::exception
    {
        public:
            // Construct
            HrException() = default;
            HrException(const HrException&) = default;
            HrException(HRESULT result, const char* what_desc, const char* what_cmd, const char* file, int line);

            // Override waht function
            const char* what() const noexcept override;

        private:
            // Raw text of exception
            std::string m_exceptionText;
    };
}
