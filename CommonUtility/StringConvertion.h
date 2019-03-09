#ifndef _STRING_CONVERTION_H_H
#define _STRING_CONVERTION_H_H

////////////////////////////////////////////////////////////////////////////
// When you not need to use CString type, you can annotate it.
#define USE_CSTRING
#define USE_ATL_SPZCE

#include <vector>
#include <string>


#ifdef USE_CSTRING

#ifdef USE_ATL_SPZCE
#include <atlstr.h>
#define XCString   CAtlString
#else
#include<afxstr.h>
#define XCString   CString
#endif



#endif


namespace common
{
	namespace text
	{
		class string_convertion
		{
		public:
			string_convertion(void);
			~string_convertion(void);

		public:

			/*
			 * convert wide string to multi bytes string.
			 * @params
			 *        lpSrc, Pointer to the Unicode string to convert.
			 *        length, the length of unicode string.
					  outLength, the length of the multi bytes string.
					  It not includes the null-terminated char.
			 * return
			 *       Success return the multi string or return NULL.
			 */
			char * WideCharToMultiChar(const wchar_t * lpSrc, int length, int *outLength);

			/*
			 * Convert wide string to utf8 string.
			 *@params
			 *       lpSrc, point to the unicode string to convert.
			 *       length, the length of unicode string.
			 *       outLength, the length of the utf8 string.
					 It not include the null-terminated char.
			 *return
					 Success return the utf8 string or return NULL.
			 */
			char * WideCharToUTF8String(const wchar_t * lpSrc, int length, int* outLength);

			/*
			 * convert multi byte string to wide string.
			 * @params
			 *        lpSrc, pointer to the character string to convert.
			 *        length, the length of character string to convert.
					  outLength, the length of the unicode string that returned.
					  It not includes the null-terminated char.
			 * return
			 *       Success return the Unicode string or return NULL.
			 */
			wchar_t * MultiCharToWideChar(const char * lpSrc, int length, int* outLength);

			/*
			 * convert multi bytes string to utf8 string.
			 *@params
			 *       lpSrc, pointer to the character string to convert.
			 *       length, the length of character string to convert.
			 *       outLength, the length of the utf8 string that returned.
					 It not include the null-terminated char.
			 *return
			 *       Success return the utf8 string or return NULL.
			 */
			char * MultiCharToUTF8String(const char * lpSrc, int length, int* outLength);

			/*
			 * Convert utf8 string to wide char string.
			 *@params
			  *      lpSrc, pointer to the utf8 string to convert.
			  *      length, the length of the utf8 string to convert.
			  *      outLength, the length of the wide char string that was returned.
					 It not include the null-terminated char.
			  *return
			  *     Success return the wide char string or return NULL.
			  */
			wchar_t * UTF8StringToWideChar(const char * lpSrc, int length, int* outLength);

			/*
			 * Convert utf8 string to multi bytes string.
			 *@params
			 *       lpSrc, pointer to the utf8 string to convert.
			 *       length, the length of the utf8 string to convert.
			 *       outLength, the length of the character string.
					 It not include the null-terminated char.
			 *return
			 *      Success return multi bytes string or return NULL.
			 */
			char * UTF8StringToMultiChar(const char * lpSrc, int length, int *outLength);

			/*
			 * convert multi bytes string to wide char string.
			 *@params
			 *       sourceStr, the source string to be convert.
			 *return
			 *      The converted wide char string.
			 */
			std::wstring MultiCharToWideChar(const std::string & sourceStr);

			/*
			 *Convert wide char string to multi bytes string.
			 *@params
			 *      sourceStr, the source string to be convert.
			 *return
			 *     The converted multi bytes string.
			 */
			std::string WideCharToMultiChar(const std::wstring & sourceStr);

			/* Convert TCHAR string to UTF8 string
			 *@params
			 *      lpSrc, a pointer to the source string.
			 *      length, the length of the source string.
			 *      outLength, a pointer to the utf8 encodeing string length.
			 */
			char * TCHARToUTF8String(const TCHAR * lpSrc, int length, int * outLength);

			/**
			 * Convert UTF8 encoding string to TCHAR string.
			 *@params
			 *
			 */

			TCHAR * UTF8StringToTCHAR(char * lpSrc, int length, int *outLength);

#ifdef USE_CSTRING
			/*
			 * Convert CString to character string.
			 *@params
			 *       src, the CString character to be converted.
			 *return
			 *      The character string.
			 */
			std::string CStringToString(XCString  src);
#endif


#ifdef USE_CSTRING
			/*
			 * Convert CString to wide char string.
			 *@params
			 *       src, the CString character to be converted.
			 *return
			 *      The wide char string.
			 */
			std::wstring CStringToWstring(XCString src);
#endif


#ifdef USE_CSTRING
			/*
			 * Convert character string to CString.
			 *@params
			 *       character string to be converted.
			 *return
			 *       The CString characters.
			 */
			XCString StringToCString(const std::string & src);
#endif


#ifdef USE_CSTRING
			/*
			 * Convert wide char string to CString.
			 *@params
			 *       wide char string to be converted.
			 *return
			 *      The CString characters.
			 */
			XCString WStringToCString(const std::wstring & src);
#endif

			/*
			* Convert utf8 string to wstring.
			*@params
			*       utf8 encoding char string to be converted.
			*return
			*      The wstring characters.
			*/
			std::wstring UTF8StringToWstring(char * lpStr);


		private:
			std::vector<char*> m_multiChar;
			std::vector<wchar_t*> m_wideChar;
			std::vector<char *> m_utf8Char;

		};

	}
}

#endif