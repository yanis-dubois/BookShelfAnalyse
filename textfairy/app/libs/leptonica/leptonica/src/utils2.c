/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*!
 * \file utils2.c
 * <pre>
 *
 *       Safe string procs
 *           char      *stringNew()
 *           l_int32    stringCopy()
 *           l_int32    stringReplace()
 *           l_int32    stringLength()
 *           l_int32    stringCat()
 *           char      *stringConcatNew()
 *           char      *stringJoin()
 *           l_int32    stringJoinIP()
 *           char      *stringReverse()
 *           char      *strtokSafe()
 *           l_int32    stringSplitOnToken()
 *
 *       Find and replace string and array procs
 *           l_int32    stringCheckForChars()
 *           char      *stringRemoveChars()
 *           l_int32    stringFindSubstr()
 *           char      *stringReplaceSubstr()
 *           char      *stringReplaceEachSubstr()
 *           L_DNA     *arrayFindEachSequence()
 *           l_int32    arrayFindSequence()
 *
 *       Safe realloc
 *           void      *reallocNew()
 *
 *       Read and write between file and memory
 *           l_uint8   *l_binaryRead()
 *           l_uint8   *l_binaryReadStream()
 *           l_uint8   *l_binaryReadSelect()
 *           l_uint8   *l_binaryReadSelectStream()
 *           l_int32    l_binaryWrite()
 *           l_int32    nbytesInFile()
 *           l_int32    fnbytesInFile()
 *
 *       Copy in memory
 *           l_uint8   *l_binaryCopy()
 *
 *       File copy operations
 *           l_int32    fileCopy()
 *           l_int32    fileConcatenate()
 *           l_int32    fileAppendString()
 *
 *       Multi-platform functions for opening file streams
 *           FILE      *fopenReadStream()
 *           FILE      *fopenWriteStream()
 *           FILE      *fopenReadFromMemory()
 *
 *       Opening a windows tmpfile for writing
 *           FILE      *fopenWriteWinTempfile()
 *
 *       Multi-platform functions that avoid C-runtime boundary crossing
 *       with Windows DLLs
 *           FILE      *lept_fopen()
 *           l_int32    lept_fclose()
 *           void       lept_calloc()
 *           void       lept_free()
 *
 *       Multi-platform file system operations in temp directories
 *           l_int32    lept_mkdir()
 *           l_int32    lept_rmdir()
 *           l_int32    lept_direxists()
 *           l_int32    lept_mv()
 *           l_int32    lept_rm_match()
 *           l_int32    lept_rm()
 *           l_int32    lept_rmfile()
 *           l_int32    lept_cp()
 *
 *       General file name operations
 *           l_int32    splitPathAtDirectory()
 *           l_int32    splitPathAtExtension()
 *           char      *pathJoin()
 *           char      *appendSubdirs()
 *
 *       Special file name operations
 *           l_int32    convertSepCharsInPath()
 *           char      *genPathname()
 *           l_int32    makeTempDirname()
 *           l_int32    modifyTrailingSlash()
 *           char      *l_makeTempFilename()
 *           l_int32    extractNumberFromFilename()
 *
 *
 *  Notes on multi-platform development
 *  -----------------------------------
 *  This is important:
 *  (1) With the exception of splitPathAtDirectory(), splitPathAtExtension()
  *     and genPathname(), all input pathnames must have unix separators.
 *  (2) On Windows, when you specify a read or write to "/tmp/...",
 *      the filename is rewritten to use the Windows temp directory:
 *         /tmp  ==>   <Temp>...    (windows)
 *  (3) This filename rewrite, along with the conversion from unix
 *      to windows pathnames, happens in genPathname().
 *  (4) Use fopenReadStream() and fopenWriteStream() to open files,
 *      because these use genPathname() to find the platform-dependent
 *      filenames.  Likewise for l_binaryRead() and l_binaryWrite().
 *  (5) For moving, copying and removing files and directories that are in
 *      subdirectories of /tmp, use the lept_*() file system shell wrappers:
 *         lept_mkdir(), lept_rmdir(), lept_mv(), lept_rm() and lept_cp().
 *  (6) Use the lept_*() C library wrappers.  These work properly on
 *      Windows, where the same DLL must perform complementary operations
 *      on file streams (open/close) and heap memory (malloc/free):
 *         lept_fopen(), lept_fclose(), lept_calloc() and lept_free().
 *  (7) Why read and write files to temp directories?
 *      The library needs the ability to read and write ephemeral
 *      files to default places, both for generating debugging output
 *      and for supporting regression tests.  Applications also need
 *      this ability for debugging.
 *  (8) Why do the pathname rewrite on Windows?
 *      The goal is to have the library, and programs using the library,
 *      run on multiple platforms without changes.  The location of
 *      temporary files depends on the platform as well as the user's
 *      configuration.  Temp files on Windows are in some directory
 *      not known a priori.  To make everything work seamlessly on
 *      Windows, every time you open a file for reading or writing,
 *      use a special function such as fopenReadStream() or
 *      fopenWriteStream(); these call genPathname() to ensure that
 *      if it is a temp file, the correct path is used.  To indicate
 *      that this is a temp file, the application is written with the
 *      root directory of the path in a canonical form: "/tmp".
 *  (9) Why is it that multi-platform directory functions like lept_mkdir()
 *      and lept_rmdir(), as well as associated file functions like
 *      lept_rm(), lept_mv() and lept_cp(), only work in the temp dir?
 *      These functions were designed to provide easy manipulation of
 *      temp files.  The restriction to temp files is for safety -- to
 *      prevent an accidental deletion of important files.  For example,
 *      lept_rmdir() first deletes all files in a specified subdirectory
 *      of temp, and then removes the directory.
 *
 * </pre>
 */

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif  /* HAVE_CONFIG_H */

#ifdef _MSC_VER
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#endif   /* _MSC_VER */

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>     /* _O_CREAT, ... */
#include <io.h>        /* _open */
#include <sys/stat.h>  /* _S_IREAD, _S_IWRITE */
#else
#include <sys/stat.h>  /* for stat, mkdir(2) */
#include <sys/types.h>
#endif

#include <string.h>
#include <stddef.h>
#include "allheaders.h"

/*  This is only used to test "/tmp" --> TMPDIR rewriting on Windows,
 *  by emulating it in unix.  It should never be on in production. */
#define DEBUG_REWRITE    0


/*--------------------------------------------------------------------*
 *                       Safe string operations                       *
 *--------------------------------------------------------------------*/
/*!
 * \brief   stringNew()
 *
 * \param[in]    src string
 * \return  dest copy of src string, or NULL on error
 */
char *
stringNew(const char  *src)
{
l_int32  len;
char    *dest;

    PROCNAME("stringNew");

    if (!src) {
        L_WARNING("src not defined\n", procName);
        return NULL;
    }

    len = strlen(src);
    if ((dest = (char *)LEPT_CALLOC(len + 1, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("dest not made", procName, NULL);

    stringCopy(dest, src, len);
    return dest;
}


/*!
 * \brief   stringCopy()
 *
 * \param[in]    dest existing byte buffer
 * \param[in]    src string [optional] can be null
 * \param[in]    n max number of characters to copy
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Relatively safe wrapper for strncpy, that checks the input,
 *          and does not complain if %src is null or %n < 1.
 *          If %n < 1, this is a no-op.
 *      (2) %dest needs to be at least %n bytes in size.
 *      (3) We don't call strncpy() because valgrind complains about
 *          use of uninitialized values.
 * </pre>
 */
l_int32
stringCopy(char        *dest,
           const char  *src,
           l_int32      n)
{
l_int32  i;

    PROCNAME("stringCopy");

    if (!dest)
        return ERROR_INT("dest not defined", procName, 1);
    if (!src || n < 1)
        return 0;

        /* Implementation of strncpy that valgrind doesn't complain about */
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return 0;
}


/*!
 * \brief   stringReplace()
 *
 * \param[out]   pdest string copy
 * \param[in]    src string [optional] can be null
 * \return  0 if OK; 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Frees any existing dest string
 *      (2) Puts a copy of src string in the dest
 *      (3) If either or both strings are null, does something reasonable.
 * </pre>
 */
l_int32
stringReplace(char       **pdest,
              const char  *src)
{
    PROCNAME("stringReplace");

    if (!pdest)
        return ERROR_INT("pdest not defined", procName, 1);

    if (*pdest)
        LEPT_FREE(*pdest);

    if (src)
        *pdest = stringNew(src);
    else
        *pdest = NULL;
    return 0;
}


/*!
 * \brief   stringLength()
 *
 * \param[in]    src string can be null or NULL-terminated string
 * \param[in]    size size of src buffer
 * \return  length of src in bytes.
 *
 * <pre>
 * Notes:
 *      (1) Safe implementation of strlen that only checks size bytes
 *          for trailing NUL.
 *      (2) Valid returned string lengths are between 0 and size - 1.
 *          If size bytes are checked without finding a NUL byte, then
 *          an error is indicated by returning size.
 * </pre>
 */
l_int32
stringLength(const char  *src,
             size_t       size)
{
l_int32  i;

    PROCNAME("stringLength");

    if (!src)
        return ERROR_INT("src not defined", procName, 0);
    if (size < 1)
        return 0;

    for (i = 0; i < size; i++) {
        if (src[i] == '\0')
            return i;
    }
    return size;  /* didn't find a NUL byte */
}


/*!
 * \brief   stringCat()
 *
 * \param[in]    dest null-terminated byte buffer
 * \param[in]    size size of dest
 * \param[in]    src string can be null or NULL-terminated string
 * \return  number of bytes added to dest; -1 on error
 *
 * <pre>
 * Notes:
 *      (1) Alternative implementation of strncat, that checks the input,
 *          is easier to use (since the size of the dest buffer is specified
 *          rather than the number of bytes to copy), and does not complain
 *          if %src is null.
 *      (2) Never writes past end of dest.
 *      (3) If it can't append src (an error), it does nothing.
 *      (4) N.B. The order of 2nd and 3rd args is reversed from that in
 *          strncat, as in the Windows function strcat_s().
 * </pre>
 */
l_int32
stringCat(char        *dest,
          size_t       size,
          const char  *src)
{
l_int32  i, n;
l_int32  lendest, lensrc;

    PROCNAME("stringCat");

    if (!dest)
        return ERROR_INT("dest not defined", procName, -1);
    if (size < 1)
        return ERROR_INT("size < 1; too small", procName, -1);
    if (!src)
        return 0;

    lendest = stringLength(dest, size);
    if (lendest == size)
        return ERROR_INT("no terminating nul byte", procName, -1);
    lensrc = stringLength(src, size);
    if (lensrc == 0)
        return 0;
    n = (lendest + lensrc > size - 1 ? size - lendest - 1 : lensrc);
    if (n < 1)
        return ERROR_INT("dest too small for append", procName, -1);

    for (i = 0; i < n; i++)
        dest[lendest + i] = src[i];
    dest[lendest + n] = '\0';
    return n;
}


/*!
 * \brief   stringConcatNew()
 *
 * \param[in]    first first string in list
 * \param[in]    ...  NULL-terminated list of strings
 * \return  result new string concatenating the input strings, or
 *                      NULL if first == NULL
 *
 * <pre>
 * Notes:
 *      (1) The last arg in the list of strings must be NULL.
 *      (2) Caller must free the returned string.
 * </pre>
 */
char *
stringConcatNew(const char  *first, ...)
{
size_t       len;
char        *result, *ptr;
const char  *arg;
va_list      args;

    if (!first) return NULL;

        /* Find the length of the output string */
    va_start(args, first);
    len = strlen(first);
    while ((arg = va_arg(args, const char *)) != NULL)
        len += strlen(arg);
    va_end(args);
    result = (char *)LEPT_CALLOC(len + 1, sizeof(char));

        /* Concatenate the args */
    va_start(args, first);
    ptr = result;
    arg = first;
    while (*arg)
        *ptr++ = *arg++;
    while ((arg = va_arg(args, const char *)) != NULL) {
        while (*arg)
            *ptr++ = *arg++;
    }
    va_end(args);
    return result;
}


/*!
 * \brief   stringJoin()
 *
 * \param[in]    src1 string [optional] can be null
 * \param[in]    src2 string [optional] can be null
 * \return  concatenated string, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) This is a safe version of strcat; it makes a new string.
 *      (2) It is not an error if either or both of the strings
 *          are empty, or if either or both of the pointers are null.
 * </pre>
 */
char *
stringJoin(const char  *src1,
           const char  *src2)
{
char    *dest;
l_int32  srclen1, srclen2, destlen;

    PROCNAME("stringJoin");

    srclen1 = (src1) ? strlen(src1) : 0;
    srclen2 = (src2) ? strlen(src2) : 0;
    destlen = srclen1 + srclen2 + 3;

    if ((dest = (char *)LEPT_CALLOC(destlen, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("calloc fail for dest", procName, NULL);

    if (src1)
        stringCopy(dest, src1, srclen1);
    if (src2)
        strncat(dest, src2, srclen2);
    return dest;
}


/*!
 * \brief   stringJoinIP()
 *
 * \param[in,out]  psrc1 string address of src1; cannot be on the stack
 * \param[in]      src2 string [optional] can be null
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This is a safe in-place version of strcat.  The contents of
 *          src1 is replaced by the concatenation of src1 and src2.
 *      (2) It is not an error if either or both of the strings
 *          are empty (""), or if the pointers to the strings (*psrc1, src2)
 *          are null.
 *      (3) src1 should be initialized to null or an empty string
 *          before the first call.  Use one of these:
 *              char *src1 = NULL;
 *              char *src1 = stringNew("");
 *          Then call with:
 *              stringJoinIP(&src1, src2);
 *      (4) This can also be implemented as a macro:
 * \code
 *              #define stringJoinIP(src1, src2) \
 *                  {tmpstr = stringJoin((src1),(src2)); \
 *                  LEPT_FREE(src1); \
 *                  (src1) = tmpstr;}
 * \endcode
 *      (5) Another function to consider for joining many strings is
 *          stringConcatNew().
 * </pre>
 */
l_int32
stringJoinIP(char       **psrc1,
             const char  *src2)
{
char  *tmpstr;

    PROCNAME("stringJoinIP");

    if (!psrc1)
        return ERROR_INT("&src1 not defined", procName, 1);

    tmpstr = stringJoin(*psrc1, src2);
    LEPT_FREE(*psrc1);
    *psrc1 = tmpstr;
    return 0;
}


/*!
 * \brief   stringReverse()
 *
 * \param[in]    src string
 * \return  dest newly-allocated reversed string
 */
char *
stringReverse(const char  *src)
{
char    *dest;
l_int32  i, len;

    PROCNAME("stringReverse");

    if (!src)
        return (char *)ERROR_PTR("src not defined", procName, NULL);
    len = strlen(src);
    if ((dest = (char *)LEPT_CALLOC(len + 1, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("calloc fail for dest", procName, NULL);
    for (i = 0; i < len; i++)
        dest[i] = src[len - 1 - i];

    return dest;
}


/*!
 * \brief   strtokSafe()
 *
 * \param[in]    cstr input string to be sequentially parsed;
 *                    use NULL after the first call
 * \param[in]    seps a string of character separators
 * \param[out]   psaveptr ptr to the next char after
 *                        the last encountered separator
 * \return  substr a new string that is copied from the previous
 *                      saveptr up to but not including the next
 *                      separator character, or NULL if end of cstr.
 *
 * <pre>
 * Notes:
 *      (1) This is a thread-safe implementation of strtok.
 *      (2) It has the same interface as strtok_r.
 *      (3) It differs from strtok_r in usage in two respects:
 *          (a) the input string is not altered
 *          (b) each returned substring is newly allocated and must
 *              be freed after use.
 *      (4) Let me repeat that.  This is "safe" because the input
 *          string is not altered and because each returned string
 *          is newly allocated on the heap.
 *      (5) It is here because, surprisingly, some C libraries don't
 *          include strtok_r.
 *      (6) Important usage points:
 *          ~ Input the string to be parsed on the first invocation.
 *          ~ Then input NULL after that; the value returned in saveptr
 *            is used in all subsequent calls.
 *      (7) This is only slightly slower than strtok_k.
 * </pre>
 */
char *
strtokSafe(char        *cstr,
           const char  *seps,
           char       **psaveptr)
{
char     nextc;
char    *start, *substr;
l_int32  istart, i, j, nchars;

    PROCNAME("strtokSafe");

    if (!seps)
        return (char *)ERROR_PTR("seps not defined", procName, NULL);
    if (!psaveptr)
        return (char *)ERROR_PTR("&saveptr not defined", procName, NULL);

    if (!cstr) {
        start = *psaveptr;
    } else {
        start = cstr;
        *psaveptr = NULL;
    }
    if (!start)  /* nothing to do */
        return NULL;

        /* First time, scan for the first non-sep character */
    istart = 0;
    if (cstr) {
        for (istart = 0;; istart++) {
            if ((nextc = start[istart]) == '\0') {
                *psaveptr = NULL;  /* in case caller doesn't check ret value */
                return NULL;
            }
            if (!strchr(seps, nextc))
                break;
        }
    }

        /* Scan through, looking for a sep character; if none is
         * found, 'i' will be at the end of the string. */
    for (i = istart;; i++) {
        if ((nextc = start[i]) == '\0')
            break;
        if (strchr(seps, nextc))
            break;
    }

        /* Save the substring */
    nchars = i - istart;
    substr = (char *)LEPT_CALLOC(nchars + 1, sizeof(char));
    stringCopy(substr, start + istart, nchars);

        /* Look for the next non-sep character.
         * If this is the last substring, return a null saveptr. */
    for (j = i;; j++) {
        if ((nextc = start[j]) == '\0') {
            *psaveptr = NULL;  /* no more non-sep characters */
            break;
        }
        if (!strchr(seps, nextc)) {
            *psaveptr = start + j;  /* start here on next call */
                break;
        }
    }

    return substr;
}


/*!
 * \brief   stringSplitOnToken()
 *
 * \param[in]    cstr input string to be split; not altered
 * \param[in]    seps a string of character separators
 * \param[out]   phead ptr to copy of the input string, up to
 *                     the first separator token encountered
 * \param[out]   ptail ptr to copy of the part of the input string
 *                     starting with the first non-separator character
 *                     that occurs after the first separator is found
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) The input string is not altered; all split parts are new strings.
 *      (2) The split occurs around the first consecutive sequence of
 *          tokens encountered.
 *      (3) The head goes from the beginning of the string up to
 *          but not including the first token found.
 *      (4) The tail contains the second part of the string, starting
 *          with the first char in that part that is NOT a token.
 *      (5) If no separator token is found, 'head' contains a copy
 *          of the input string and 'tail' is null.
 * </pre>
 */
l_int32
stringSplitOnToken(char        *cstr,
                   const char  *seps,
                   char       **phead,
                   char       **ptail)
{
char  *saveptr;

    PROCNAME("stringSplitOnToken");

    if (!phead)
        return ERROR_INT("&head not defined", procName, 1);
    if (!ptail)
        return ERROR_INT("&tail not defined", procName, 1);
    *phead = *ptail = NULL;
    if (!cstr)
        return ERROR_INT("cstr not defined", procName, 1);
    if (!seps)
        return ERROR_INT("seps not defined", procName, 1);

    *phead = strtokSafe(cstr, seps, &saveptr);
    if (saveptr)
        *ptail = stringNew(saveptr);
    return 0;
}


/*--------------------------------------------------------------------*
 *                       Find and replace procs                       *
 *--------------------------------------------------------------------*/
/*!
 * \brief   stringCheckForChars()
 *
 * \param[in]    src      input string; can be of zero length
 * \param[in]    chars    string of chars to be searched for in %src
 * \param[out]   pfound   1 if any characters are found; 0 otherwise
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This can be used to sanitize an operation by checking for
 *          special characters that don't belong in a string.
 * </pre>
 */
l_int32
stringCheckForChars(const char  *src,
                    const char  *chars,
                    l_int32     *pfound)
{
char     ch;
l_int32  i, n;

    PROCNAME("stringCheckForChars");

    if (!pfound)
        return ERROR_INT("&found not defined", procName, 1);
    *pfound = FALSE;
    if (!src || !chars)
        return ERROR_INT("src and chars not both defined", procName, 1);

    n = strlen(src);
    for (i = 0; i < n; i++) {
        ch = src[i];
        if (strchr(chars, ch)) {
            *pfound = TRUE;
            break;
        }
    }
    return 0;
}


/*!
 * \brief   stringRemoveChars()
 *
 * \param[in]    src input string; can be of zero length
 * \param[in]    remchars  string of chars to be removed from src
 * \return  dest string with specified chars removed, or NULL on error
 */
char *
stringRemoveChars(const char  *src,
                  const char  *remchars)
{
char     ch;
char    *dest;
l_int32  nsrc, i, k;

    PROCNAME("stringRemoveChars");

    if (!src)
        return (char *)ERROR_PTR("src not defined", procName, NULL);
    if (!remchars)
        return stringNew(src);

    if ((dest = (char *)LEPT_CALLOC(strlen(src) + 1, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("dest not made", procName, NULL);
    nsrc = strlen(src);
    for (i = 0, k = 0; i < nsrc; i++) {
        ch = src[i];
        if (!strchr(remchars, ch))
            dest[k++] = ch;
    }

    return dest;
}


/*!
 * \brief   stringFindSubstr()
 *
 * \param[in]    src input string; can be of zero length
 * \param[in]    sub substring to be searched for
 * \param[out]   ploc [optional] location of substring in src
 * \return  1 if found; 0 if not found or on error
 *
 * <pre>
 * Notes:
 *      (1) This is a wrapper around strstr().
 *      (2) Both %src and %sub must be defined, and %sub must have
 *          length of at least 1.
 *      (3) If the substring is not found and loc is returned, it has
 *          the value -1.
 * </pre>
 */
l_int32
stringFindSubstr(const char  *src,
                 const char  *sub,
                 l_int32     *ploc)
{
char  *ptr;

    PROCNAME("stringFindSubstr");

    if (!src)
        return ERROR_INT("src not defined", procName, 0);
    if (!sub)
        return ERROR_INT("sub not defined", procName, 0);
    if (ploc) *ploc = -1;
    if (strlen(sub) == 0)
        return ERROR_INT("substring length 0", procName, 0);
    if (strlen(src) == 0)
        return 0;

    if ((ptr = (char *)strstr(src, sub)) == NULL)  /* not found */
        return 0;

    if (ploc)
        *ploc = ptr - src;
    return 1;
}


/*!
 * \brief   stringReplaceSubstr()
 *
 * \param[in]    src input string; can be of zero length
 * \param[in]    sub1 substring to be replaced
 * \param[in]    sub2 substring to put in; can be ""
 * \param[out]   pfound [optional] 1 if sub1 is found; 0 otherwise
 * \param[out]   ploc [optional] location of ptr after replacement
 * \return  dest string with substring replaced, or NULL if the
 *              substring not found or on error.
 *
 * <pre>
 * Notes:
 *      (1) Replaces the first instance.
 *      (2) To only remove sub1, use "" for sub2
 *      (3) Returns a new string if sub1 and sub2 are the same.
 *      (4) The optional loc is input as the byte offset within the src
 *          from which the search starts, and after the search it is the
 *          char position in the string of the next character after
 *          the substituted string.
 *      (5) N.B. If ploc is not null, loc must always be initialized.
 *          To search the string from the beginning, set loc = 0.
 * </pre>
 */
char *
stringReplaceSubstr(const char  *src,
                    const char  *sub1,
                    const char  *sub2,
                    l_int32     *pfound,
                    l_int32     *ploc)
{
char    *ptr, *dest;
l_int32  nsrc, nsub1, nsub2, len, npre, loc;

    PROCNAME("stringReplaceSubstr");

    if (!src)
        return (char *)ERROR_PTR("src not defined", procName, NULL);
    if (!sub1)
        return (char *)ERROR_PTR("sub1 not defined", procName, NULL);
    if (!sub2)
        return (char *)ERROR_PTR("sub2 not defined", procName, NULL);

    if (pfound)
        *pfound = 0;
    if (ploc)
        loc = *ploc;
    else
        loc = 0;
    if ((ptr = (char *)strstr(src + loc, sub1)) == NULL) {
        return NULL;
    }

    if (pfound)
        *pfound = 1;
    nsrc = strlen(src);
    nsub1 = strlen(sub1);
    nsub2 = strlen(sub2);
    len = nsrc + nsub2 - nsub1;
    if ((dest = (char *)LEPT_CALLOC(len + 1, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("dest not made", procName, NULL);
    npre = ptr - src;
    memcpy(dest, src, npre);
    strcpy(dest + npre, sub2);
    strcpy(dest + npre + nsub2, ptr + nsub1);
    if (ploc)
        *ploc = npre + nsub2;

    return dest;
}


/*!
 * \brief   stringReplaceEachSubstr()
 *
 * \param[in]    src input string; can be of zero length
 * \param[in]    sub1 substring to be replaced
 * \param[in]    sub2 substring to put in; can be ""
 * \param[out]   pcount [optional] the number of times that sub1
 *                      is found in src; 0 if not found
 * \return  dest string with substring replaced, or NULL if the
 *              substring not found or on error.
 *
 * <pre>
 * Notes:
 *      (1) Replaces every instance.
 *      (2) To only remove each instance of sub1, use "" for sub2
 *      (3) Returns NULL if sub1 and sub2 are the same.
 * </pre>
 */
char *
stringReplaceEachSubstr(const char  *src,
                        const char  *sub1,
                        const char  *sub2,
                        l_int32     *pcount)
{
char    *currstr, *newstr;
l_int32  loc;

    PROCNAME("stringReplaceEachSubstr");

    if (pcount) *pcount = 0;
    if (!src)
        return (char *)ERROR_PTR("src not defined", procName, NULL);
    if (!sub1)
        return (char *)ERROR_PTR("sub1 not defined", procName, NULL);
    if (!sub2)
        return (char *)ERROR_PTR("sub2 not defined", procName, NULL);

    loc = 0;
    if ((newstr = stringReplaceSubstr(src, sub1, sub2, NULL, &loc)) == NULL)
        return NULL;

    if (pcount)
        (*pcount)++;
    while (1) {
        currstr = newstr;
        newstr = stringReplaceSubstr(currstr, sub1, sub2, NULL, &loc);
        if (!newstr)
            return currstr;
        LEPT_FREE(currstr);
        if (pcount)
            (*pcount)++;
    }
}


/*!
 * \brief   arrayFindEachSequence()
 *
 * \param[in]    data byte array
 * \param[in]    datalen length of data, in bytes
 * \param[in]    sequence subarray of bytes to find in data
 * \param[in]    seqlen length of sequence, in bytes
 * \return  dna of offsets where the sequence is found, or NULL if
 *              none are found or on error
 *
 * <pre>
 * Notes:
 *      (1) The byte arrays %data and %sequence are not C strings,
 *          as they can contain null bytes.  Therefore, for each
 *          we must give the length of the array.
 *      (2) This finds every occurrence in %data of %sequence.
 * </pre>
 */
L_DNA *
arrayFindEachSequence(const l_uint8  *data,
                      size_t          datalen,
                      const l_uint8  *sequence,
                      size_t          seqlen)
{
l_int32  start, offset, realoffset, found;
L_DNA   *da;

    PROCNAME("arrayFindEachSequence");

    if (!data || !sequence)
        return (L_DNA *)ERROR_PTR("data & sequence not both defined",
                                  procName, NULL);

    da = l_dnaCreate(0);
    start = 0;
    while (1) {
        arrayFindSequence(data + start, datalen - start, sequence, seqlen,
                          &offset, &found);
        if (found == FALSE)
            break;

        realoffset = start + offset;
        l_dnaAddNumber(da, realoffset);
        start = realoffset + seqlen;
        if (start >= datalen)
            break;
    }

    if (l_dnaGetCount(da) == 0)
        l_dnaDestroy(&da);
    return da;
}


/*!
 * \brief   arrayFindSequence()
 *
 * \param[in]    data byte array
 * \param[in]    datalen length of data, in bytes
 * \param[in]    sequence subarray of bytes to find in data
 * \param[in]    seqlen length of sequence, in bytes
 * \param[out]   poffset offset from beginning of
 *                       data where the sequence begins
 * \param[out]   pfound 1 if sequence is found; 0 otherwise
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) The byte arrays 'data' and 'sequence' are not C strings,
 *          as they can contain null bytes.  Therefore, for each
 *          we must give the length of the array.
 *      (2) This searches for the first occurrence in %data of %sequence,
 *          which consists of %seqlen bytes.  The parameter %seqlen
 *          must not exceed the actual length of the %sequence byte array.
 *      (3) If the sequence is not found, the offset will be 0, so you
 *          must check %found.
 * </pre>
 */
l_int32
arrayFindSequence(const l_uint8  *data,
                  size_t          datalen,
                  const l_uint8  *sequence,
                  size_t          seqlen,
                  l_int32        *poffset,
                  l_int32        *pfound)
{
l_int32  i, j, found, lastpos;

    PROCNAME("arrayFindSequence");

    if (poffset) *poffset = 0;
    if (pfound) *pfound = FALSE;
    if (!data || !sequence)
        return ERROR_INT("data & sequence not both defined", procName, 1);
    if (!poffset || !pfound)
        return ERROR_INT("&offset and &found not defined", procName, 1);

    lastpos = datalen - seqlen + 1;
    found = FALSE;
    for (i = 0; i < lastpos; i++) {
        for (j = 0; j < seqlen; j++) {
            if (data[i + j] != sequence[j])
                 break;
            if (j == seqlen - 1)
                 found = TRUE;
        }
        if (found == TRUE)
            break;
    }

    if (found == TRUE) {
        *poffset = i;
        *pfound = TRUE;
    }
    return 0;
}


/*--------------------------------------------------------------------*
 *                             Safe realloc                           *
 *--------------------------------------------------------------------*/
/*!
 * \brief   reallocNew()
 *
 * \param[in,out]  pindata [optional]; nulls indata
 * \param[in]      oldsize size of input data to be copied, in bytes
 * \param[in]      newsize size of data to be reallocated in bytes
 * \return  ptr to new data, or NULL on error
 *
 *  Action: !N.B. 3) and (4!
 *      1 Allocates memory, initialized to 0
 *      2 Copies as much of the input data as possible
 *          to the new block, truncating the copy if necessary
 *      3 Frees the input data
 *      4 Zeroes the input data ptr
 *
 * <pre>
 * Notes:
 *      (1) If newsize <=0, just frees input data and nulls ptr
 *      (2) If input ptr is null, just callocs new memory
 *      (3) This differs from realloc in that it always allocates
 *          new memory (if newsize > 0) and initializes it to 0,
 *          it requires the amount of old data to be copied,
 *          and it takes the address of the input ptr and
 *          nulls the handle.
 * </pre>
 */
void *
reallocNew(void   **pindata,
           l_int32  oldsize,
           l_int32  newsize)
{
l_int32  minsize;
void    *indata;
void    *newdata;

    PROCNAME("reallocNew");

    if (!pindata)
        return ERROR_PTR("input data not defined", procName, NULL);
    indata = *pindata;

    if (newsize <= 0) {   /* nonstandard usage */
        if (indata) {
            LEPT_FREE(indata);
            *pindata = NULL;
        }
        return NULL;
    }

    if (!indata) {  /* nonstandard usage */
        if ((newdata = (void *)LEPT_CALLOC(1, newsize)) == NULL)
            return ERROR_PTR("newdata not made", procName, NULL);
        return newdata;
    }

        /* Standard usage */
    if ((newdata = (void *)LEPT_CALLOC(1, newsize)) == NULL)
        return ERROR_PTR("newdata not made", procName, NULL);
    minsize = L_MIN(oldsize, newsize);
    memcpy((char *)newdata, (char *)indata, minsize);

    LEPT_FREE(indata);
    *pindata = NULL;

    return newdata;
}


/*--------------------------------------------------------------------*
 *                 Read and write between file and memory             *
 *--------------------------------------------------------------------*/
/*!
 * \brief   l_binaryRead()
 *
 * \param[in]    filename
 * \param[out]   pnbytes number of bytes read
 * \return  data, or NULL on error
 */
l_uint8 *
l_binaryRead(const char  *filename,
             size_t      *pnbytes)
{
l_uint8  *data;
FILE     *fp;

    PROCNAME("l_binaryRead");

    if (!pnbytes)
        return (l_uint8 *)ERROR_PTR("pnbytes not defined", procName, NULL);
    *pnbytes = 0;
    if (!filename)
        return (l_uint8 *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (l_uint8 *)ERROR_PTR("file stream not opened", procName, NULL);
    data = l_binaryReadStream(fp, pnbytes);
    fclose(fp);
    return data;
}


/*!
 * \brief   l_binaryReadStream()
 *
 * \param[in]    fp file stream opened to read; can be stdin
 * \param[out]   pnbytes number of bytes read
 * \return  null-terminated array, or NULL on error
 *              reading 0 bytes is not an error
 *
 * <pre>
 * Notes:
 *      (1) The returned array is terminated with a null byte so that it can
 *          be used to read ascii data from a file into a proper C string.
 *      (2) This can be used to capture data that is piped in via stdin,
 *          because it does not require seeking within the file.
 *      (3) For example, you can read an image from stdin into memory
 *          using shell redirection, with one of these shell commands:
 *             cat <imagefile> | readprog
 *             readprog < <imagefile>
 *          where readprog is:
 *             l_uint8 *data = l_binaryReadStream(stdin, &nbytes);
 *             Pix *pix = pixReadMem(data, nbytes);
 * </pre>
 */
l_uint8 *
l_binaryReadStream(FILE    *fp,
                   size_t  *pnbytes)
{
l_uint8    *data;
l_int32     seekable, navail, nadd, nread;
L_BBUFFER  *bb;

    PROCNAME("l_binaryReadStream");

    if (!pnbytes)
        return (l_uint8 *)ERROR_PTR("&nbytes not defined", procName, NULL);
    *pnbytes = 0;
    if (!fp)
        return (l_uint8 *)ERROR_PTR("fp not defined", procName, NULL);

        /* Test if the stream is seekable, by attempting to seek to
         * the start of data.  This is a no-op.  If it is seekable, use
         * l_binaryReadSelectStream() to determine the size of the
         * data to be read in advance. */
    seekable = (ftell(fp) == 0) ? 1 : 0;
    if (seekable)
        return l_binaryReadSelectStream(fp, 0, 0, pnbytes);

        /* If it is not seekable, use the bbuffer to realloc memory
         * as needed during reading. */
    bb = bbufferCreate(NULL, 4096);
    while (1) {
        navail = bb->nalloc - bb->n;
        if (navail < 4096) {
             nadd = L_MAX(bb->nalloc, 4096);
             bbufferExtendArray(bb, nadd);
        }
        nread = fread((void *)(bb->array + bb->n), 1, 4096, fp);
        bb->n += nread;
        if (nread != 4096) break;
    }

        /* Copy the data to a new array sized for the data, because
         * the bbuffer array can be nearly twice the size we need. */
    if ((data = (l_uint8 *)LEPT_CALLOC(bb->n + 1, sizeof(l_uint8))) != NULL) {
        memcpy(data, bb->array, bb->n);
        *pnbytes = bb->n;
    } else {
        L_ERROR("calloc fail for data\n", procName);
    }

    bbufferDestroy(&bb);
    return data;
}


/*!
 * \brief   l_binaryReadSelect()
 *
 * \param[in]    filename
 * \param[in]    start first byte to read
 * \param[in]    nbytes number of bytes to read; use 0 to read to end of file
 * \param[out]   pnread number of bytes actually read
 * \return  data, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) The returned array is terminated with a null byte so that it can
 *          be used to read ascii data from a file into a proper C string.
 * </pre>
 */
l_uint8 *
l_binaryReadSelect(const char  *filename,
                   size_t       start,
                   size_t       nbytes,
                   size_t      *pnread)
{
l_uint8  *data;
FILE     *fp;

    PROCNAME("l_binaryReadSelect");

    if (!pnread)
        return (l_uint8 *)ERROR_PTR("pnread not defined", procName, NULL);
    *pnread = 0;
    if (!filename)
        return (l_uint8 *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (l_uint8 *)ERROR_PTR("file stream not opened", procName, NULL);
    data = l_binaryReadSelectStream(fp, start, nbytes, pnread);
    fclose(fp);
    return data;
}


/*!
 * \brief   l_binaryReadSelectStream()
 *
 * \param[in]    fp file stream
 * \param[in]    start first byte to read
 * \param[in]    nbytes number of bytes to read; use 0 to read to end of file
 * \param[out]   pnread number of bytes actually read
 * \return  null-terminated array, or NULL on error
 *              reading 0 bytes is not an error
 *
 * <pre>
 * Notes:
 *      (1) The returned array is terminated with a null byte so that it can
 *          be used to read ascii data from a file into a proper C string.
 *          If the file to be read is empty and %start == 0, an array
 *          with a single null byte is returned.
 *      (2) Side effect: the stream pointer is re-positioned to the
 *          beginning of the file.
 * </pre>
 */
l_uint8 *
l_binaryReadSelectStream(FILE    *fp,
                         size_t   start,
                         size_t   nbytes,
                         size_t  *pnread)
{
l_uint8  *data;
size_t    bytesleft, bytestoread, nread, filebytes;

    PROCNAME("l_binaryReadSelectStream");

    if (!pnread)
        return (l_uint8 *)ERROR_PTR("&nread not defined", procName, NULL);
    *pnread = 0;
    if (!fp)
        return (l_uint8 *)ERROR_PTR("stream not defined", procName, NULL);

        /* Verify and adjust the parameters if necessary */
    fseek(fp, 0, SEEK_END);  /* EOF */
    filebytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (start > filebytes) {
        L_ERROR("start = %lu but filebytes = %lu\n", procName,
                (unsigned long)start, (unsigned long)filebytes);
        return NULL;
    }
    if (filebytes == 0)  /* start == 0; nothing to read; return null byte */
        return (l_uint8 *)LEPT_CALLOC(1, 1);
    bytesleft = filebytes - start;  /* greater than 0 */
    if (nbytes == 0) nbytes = bytesleft;
    bytestoread = (bytesleft >= nbytes) ? nbytes : bytesleft;

        /* Read the data */
    if ((data = (l_uint8 *)LEPT_CALLOC(1, bytestoread + 1)) == NULL)
        return (l_uint8 *)ERROR_PTR("calloc fail for data", procName, NULL);
    fseek(fp, start, SEEK_SET);
    nread = fread(data, 1, bytestoread, fp);
    if (nbytes != nread)
        L_INFO("%lu bytes requested; %lu bytes read\n", procName,
               (unsigned long)nbytes, (unsigned long)nread);
    *pnread = nread;
    fseek(fp, 0, SEEK_SET);
    return data;
}


/*!
 * \brief   l_binaryWrite()
 *
 * \param[in]    filename output
 * \param[in]    operation  "w" for write; "a" for append
 * \param[in]    data  binary data to be written
 * \param[in]    nbytes  size of data array
 * \return  0 if OK; 1 on error
 */
l_int32
l_binaryWrite(const char  *filename,
              const char  *operation,
              void        *data,
              size_t       nbytes)
{
char   actualOperation[20];
FILE  *fp;

    PROCNAME("l_binaryWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!operation)
        return ERROR_INT("operation not defined", procName, 1);
    if (!data)
        return ERROR_INT("data not defined", procName, 1);
    if (nbytes <= 0)
        return ERROR_INT("nbytes must be > 0", procName, 1);

    if (strcmp(operation, "w") && strcmp(operation, "a"))
        return ERROR_INT("operation not one of {'w','a'}", procName, 1);

        /* The 'b' flag to fopen() is ignored for all POSIX
         * conforming systems.  However, Windows needs the 'b' flag. */
    stringCopy(actualOperation, operation, 2);
    strncat(actualOperation, "b", 2);

    if ((fp = fopenWriteStream(filename, actualOperation)) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    fwrite(data, 1, nbytes, fp);
    fclose(fp);
    return 0;
}


/*!
 * \brief   nbytesInFile()
 *
 * \param[in]    filename
 * \return  nbytes in file; 0 on error
 */
size_t
nbytesInFile(const char  *filename)
{
size_t  nbytes;
FILE   *fp;

    PROCNAME("nbytesInFile");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 0);
    if ((fp = fopenReadStream(filename)) == NULL)
        return ERROR_INT("stream not opened", procName, 0);
    nbytes = fnbytesInFile(fp);
    fclose(fp);
    return nbytes;
}


/*!
 * \brief   fnbytesInFile()
 *
 * \param[in]    fp file stream
 * \return  nbytes in file; 0 on error
 */
size_t
fnbytesInFile(FILE  *fp)
{
l_int64  pos, nbytes;

    PROCNAME("fnbytesInFile");

    if (!fp)
        return ERROR_INT("stream not open", procName, 0);

    pos = ftell(fp);          /* initial position */
    fseek(fp, 0, SEEK_END);   /* EOF */
    nbytes = ftell(fp);
    fseek(fp, pos, SEEK_SET);        /* back to initial position */
    return nbytes;
}


/*--------------------------------------------------------------------*
 *                            Copy in memory                          *
 *--------------------------------------------------------------------*/
/*!
 * \brief   l_binaryCopy()
 *
 * \param[in]    datas
 * \param[in]    size of data array
 * \return  datad on heap, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) We add 4 bytes to the zeroed output because in some cases
 *          (e.g., string handling) it is important to have the data
 *          be null terminated.  This guarantees that after the memcpy,
 *          the result is automatically null terminated.
 * </pre>
 */
l_uint8 *
l_binaryCopy(l_uint8  *datas,
             size_t    size)
{
l_uint8  *datad;

    PROCNAME("l_binaryCopy");

    if (!datas)
        return (l_uint8 *)ERROR_PTR("datas not defined", procName, NULL);

    if ((datad = (l_uint8 *)LEPT_CALLOC(size + 4, sizeof(l_uint8))) == NULL)
        return (l_uint8 *)ERROR_PTR("datad not made", procName, NULL);
    memcpy(datad, datas, size);
    return datad;
}


/*--------------------------------------------------------------------*
 *                         File copy operations                       *
 *--------------------------------------------------------------------*/
/*!
 * \brief   fileCopy()
 *
 * \param[in]    srcfile copy this file
 * \param[in]    newfile to this file
 * \return  0 if OK, 1 on error
 */
l_int32
fileCopy(const char  *srcfile,
         const char  *newfile)
{
l_int32   ret;
size_t    nbytes;
l_uint8  *data;

    PROCNAME("fileCopy");

    if (!srcfile)
        return ERROR_INT("srcfile not defined", procName, 1);
    if (!newfile)
        return ERROR_INT("newfile not defined", procName, 1);

    if ((data = l_binaryRead(srcfile, &nbytes)) == NULL)
        return ERROR_INT("data not returned", procName, 1);
    ret = l_binaryWrite(newfile, "w", data, nbytes);
    LEPT_FREE(data);
    return ret;
}


/*!
 * \brief   fileConcatenate()
 *
 * \param[in]    srcfile file to append
 * \param[in]    destfile file to add to
 * \return  0 if OK, 1 on error
 */
l_int32
fileConcatenate(const char  *srcfile,
                const char  *destfile)
{
size_t    nbytes;
l_uint8  *data;

    PROCNAME("fileConcatenate");

    if (!srcfile)
        return ERROR_INT("srcfile not defined", procName, 1);
    if (!destfile)
        return ERROR_INT("destfile not defined", procName, 1);

    data = l_binaryRead(srcfile, &nbytes);
    l_binaryWrite(destfile, "a", data, nbytes);
    LEPT_FREE(data);
    return 0;
}


/*!
 * \brief   fileAppendString()
 *
 * \param[in]    filename
 * \param[in]    str string to append to file
 * \return  0 if OK, 1 on error
 */
l_int32
fileAppendString(const char  *filename,
                 const char  *str)
{
FILE  *fp;

    PROCNAME("fileAppendString");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!str)
        return ERROR_INT("str not defined", procName, 1);

    if ((fp = fopenWriteStream(filename, "a")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    fprintf(fp, "%s", str);
    fclose(fp);
    return 0;
}


/*--------------------------------------------------------------------*
 *          Multi-platform functions for opening file streams         *
 *--------------------------------------------------------------------*/
/*!
 * \brief   fopenReadStream()
 *
 * \param[in]    filename
 * \return  stream, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) This should be used whenever you want to run fopen() to
 *          read from a stream.  Never call fopen() directory.
 *      (2) This handles the temp directory pathname conversion on windows:
 *              /tmp  ==>  <Windows Temp directory>
 * </pre>
 */
FILE *
fopenReadStream(const char  *filename)
{
char  *fname, *tail;
FILE  *fp;

    PROCNAME("fopenReadStream");

    if (!filename)
        return (FILE *)ERROR_PTR("filename not defined", procName, NULL);

        /* Try input filename */
    fname = genPathname(filename, NULL);
    fp = fopen(fname, "rb");
    LEPT_FREE(fname);
    if (fp) return fp;

        /* Else, strip directory and try locally */
    splitPathAtDirectory(filename, NULL, &tail);
    fp = fopen(tail, "rb");
    LEPT_FREE(tail);

    if (!fp)
        return (FILE *)ERROR_PTR("file not found", procName, NULL);
    return fp;
}


/*!
 * \brief   fopenWriteStream()
 *
 * \param[in]    filename
 * \param[in]    modestring
 * \return  stream, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) This should be used whenever you want to run fopen() to
 *          write or append to a stream.  Never call fopen() directory.
 *      (2) This handles the temp directory pathname conversion on windows:
 *              /tmp  ==>  <Windows Temp directory>
 * </pre>
 */
FILE *
fopenWriteStream(const char  *filename,
                 const char  *modestring)
{
char  *fname;
FILE  *fp;

    PROCNAME("fopenWriteStream");

    if (!filename)
        return (FILE *)ERROR_PTR("filename not defined", procName, NULL);

    fname = genPathname(filename, NULL);
    fp = fopen(fname, modestring);
    LEPT_FREE(fname);
    if (!fp)
        return (FILE *)ERROR_PTR("stream not opened", procName, NULL);
    return fp;
}


/*!
 * \brief   fopenReadFromMemory()
 *
 * \param[in]    data, size
 * \return  file stream, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) Work-around if fmemopen() not available.
 *      (2) Windows tmpfile() writes into the root C:\ directory, which
 *          requires admin privileges.  This also works around that.
 * </pre>
 */
FILE *
fopenReadFromMemory(const l_uint8  *data,
                    size_t          size)
{
FILE  *fp;

    PROCNAME("fopenReadFromMemory");

    if (!data)
        return (FILE *)ERROR_PTR("data not defined", procName, NULL);

#if HAVE_FMEMOPEN
    if ((fp = fmemopen((void *)data, size, "rb")) == NULL)
        return (FILE *)ERROR_PTR("stream not opened", procName, NULL);
#else  /* write to tmp file */
    L_INFO("work-around: writing to a temp file\n", procName);
  #ifdef _WIN32
    if ((fp = fopenWriteWinTempfile()) == NULL)
        return (FILE *)ERROR_PTR("tmpfile stream not opened", procName, NULL);
  #else
    if ((fp = tmpfile()) == NULL)
        return (FILE *)ERROR_PTR("tmpfile stream not opened", procName, NULL);
  #endif  /*  _WIN32 */
    fwrite(data, 1, size, fp);
    rewind(fp);
#endif  /* HAVE_FMEMOPEN */

    return fp;
}


/*--------------------------------------------------------------------*
 *                Opening a windows tmpfile for writing               *
 *--------------------------------------------------------------------*/
/*!
 * \brief   fopenWriteWinTempfile()
 *
 * \return  file stream, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) The Windows version of tmpfile() writes into the root
 *          C:\ directory, which requires admin privileges.  This
 *          function provides an alternative implementation.
 * </pre>
 */
FILE *
fopenWriteWinTempfile()
{
#ifdef _WIN32
l_int32  handle;
FILE    *fp;
char    *filename;

    PROCNAME("fopenWriteWinTempfile");

    if ((filename = l_makeTempFilename()) == NULL) {
        L_ERROR("l_makeTempFilename failed, %s\n", procName, strerror(errno));
        return NULL;
    }

    handle = _open(filename, _O_CREAT | _O_RDWR | _O_SHORT_LIVED |
                   _O_TEMPORARY | _O_BINARY, _S_IREAD | _S_IWRITE);
    lept_free(filename);
    if (handle == -1) {
        L_ERROR("_open failed, %s\n", procName, strerror(errno));
        return NULL;
    }

    if ((fp = _fdopen(handle, "r+b")) == NULL) {
        L_ERROR("_fdopen failed, %s\n", procName, strerror(errno));
        return NULL;
    }

    return fp;
#else
    return NULL;
#endif  /*  _WIN32 */
}


/*--------------------------------------------------------------------*
 *       Multi-platform functions that avoid C-runtime boundary       *
 *             crossing for applications with Windows DLLs            *
 *--------------------------------------------------------------------*/
/*
 *  Problems arise when pointers to streams and data are passed
 *  between two Windows DLLs that have been generated with different
 *  C runtimes.  To avoid this, leptonica provides wrappers for
 *  several C library calls.
 */
/*!
 * \brief   lept_fopen()
 *
 * \param[in]    filename
 * \param[in]    mode same as for fopen(); e.g., "rb"
 * \return  stream or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) This must be used by any application that passes
 *          a file handle to a leptonica Windows DLL.
 * </pre>
 */
FILE *
lept_fopen(const char  *filename,
           const char  *mode)
{
    PROCNAME("lept_fopen");

    if (!filename)
        return (FILE *)ERROR_PTR("filename not defined", procName, NULL);
    if (!mode)
        return (FILE *)ERROR_PTR("mode not defined", procName, NULL);

    if (stringFindSubstr(mode, "r", NULL))
        return fopenReadStream(filename);
    else
        return fopenWriteStream(filename, mode);
}


/*!
 * \brief   lept_fclose()
 *
 * \param[in]    fp file stream
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This should be used by any application that accepts
 *          a file handle generated by a leptonica Windows DLL.
 * </pre>
 */
l_int32
lept_fclose(FILE *fp)
{
    PROCNAME("lept_fclose");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);

    return fclose(fp);
}


/*!
 * \brief   lept_calloc()
 *
 * \param[in]    nmemb number of members
 * \param[in]    size of each member
 * \return  void ptr, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) For safety with windows DLLs, this can be used in conjunction
 *          with lept_free() to avoid C-runtime boundary problems.
 *          Just use these two functions throughout your application.
 * </pre>
 */
void *
lept_calloc(size_t  nmemb,
            size_t  size)
{
    if (nmemb <= 0 || size <= 0)
        return NULL;
    return LEPT_CALLOC(nmemb, size);
}


/*!
 * \brief   lept_free()
 *
 * \param[in]    ptr
 *
 * <pre>
 * Notes:
 *      (1) This should be used by any application that accepts
 *          heap data allocated by a leptonica Windows DLL.
 * </pre>
 */
void
lept_free(void *ptr)
{
    if (!ptr) return;
    LEPT_FREE(ptr);
    return;
}


/*--------------------------------------------------------------------*
 *                Multi-platform file system operations               *
 *         [ These only write to /tmp or its subdirectories ]         *
 *--------------------------------------------------------------------*/
/*!
 * \brief   lept_mkdir()
 *
 * \param[in]    subdir of /tmp or its equivalent on Windows
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) %subdir is a partial path that can consist of one or more
 *          directories.
 *      (2) This makes any subdirectories of /tmp that are required.
 *      (3) The root temp directory is:
 *            /tmp    (unix)  [default]
 *            <Temp>  (windows)
 * </pre>
 */
l_int32
lept_mkdir(const char  *subdir)
{
char     *dir, *tmpdir;
l_int32   i, n;
l_int32   ret = 0;
SARRAY   *sa;
#ifdef  _WIN32
l_uint32  attributes;
#endif  /* _WIN32 */

    PROCNAME("lept_mkdir");

    if (!subdir)
        return ERROR_INT("subdir not defined", procName, 1);
    if ((strlen(subdir) == 0) || (subdir[0] == '.') || (subdir[0] == '/'))
        return ERROR_INT("subdir not an actual subdirectory", procName, 1);

    sa = sarrayCreate(0);
    sarraySplitString(sa, subdir, "/");
    n = sarrayGetCount(sa);
    dir = genPathname("/tmp", NULL);
       /* Make sure the tmp directory exists */
#ifndef _WIN32
    ret = mkdir(dir, 0777);
#else
    attributes = GetFileAttributes(dir);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        ret = (CreateDirectory(dir, NULL) ? 0 : 1);
#endif
        /* Make all the subdirectories */
    for (i = 0; i < n; i++) {
        tmpdir = pathJoin(dir, sarrayGetString(sa, i, L_NOCOPY));
#ifndef _WIN32
        ret += mkdir(tmpdir, 0777);
#else
        if (CreateDirectory(tmpdir, NULL) == 0)
            ret += (GetLastError () != ERROR_ALREADY_EXISTS);
#endif
        LEPT_FREE(dir);
        dir = tmpdir;
    }
    LEPT_FREE(dir);
    sarrayDestroy(&sa);
    if (ret > 0)
        L_ERROR("failure to create %d directories\n", procName, ret);
    return ret;
}


/*!
 * \brief   lept_rmdir()
 *
 * \param[in]    subdir of /tmp or its equivalent on Windows
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) %subdir is a partial path that can consist of one or more
 *          directories.
 *      (2) This removes all files from the specified subdirectory of
 *          the root temp directory:
 *            /tmp    (unix)
 *            <Temp>  (windows)
 *          and then removes the subdirectory.
 *      (3) The combination
 *            lept_rmdir(subdir);
 *            lept_mkdir(subdir);
 *          is guaranteed to give you an empty subdirectory.
 * </pre>
 */
l_int32
lept_rmdir(const char  *subdir)
{
char    *dir, *realdir, *fname, *fullname;
l_int32  exists, ret, i, nfiles;
SARRAY  *sa;
#ifdef _WIN32
char    *newpath;
#endif  /* _WIN32 */

    PROCNAME("lept_rmdir");

    if (!subdir)
        return ERROR_INT("subdir not defined", procName, 1);
    if ((strlen(subdir) == 0) || (subdir[0] == '.') || (subdir[0] == '/'))
        return ERROR_INT("subdir not an actual subdirectory", procName, 1);

        /* Find the temp subdirectory */
    dir = pathJoin("/tmp", subdir);
    if (!dir)
        return ERROR_INT("directory name not made", procName, 1);
    lept_direxists(dir, &exists);
    if (!exists) {  /* fail silently */
        LEPT_FREE(dir);
        return 0;
    }

        /* List all the files in that directory */
    if ((sa = getFilenamesInDirectory(dir)) == NULL) {
        L_ERROR("directory %s does not exist!\n", procName, dir);
        LEPT_FREE(dir);
        return 1;
    }
    nfiles = sarrayGetCount(sa);

    for (i = 0; i < nfiles; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        fullname = genPathname(dir, fname);
        remove(fullname);
        LEPT_FREE(fullname);
    }

#ifndef _WIN32
    realdir = genPathname("/tmp", subdir);
    ret = rmdir(realdir);
    LEPT_FREE(realdir);
#else
    newpath = genPathname(dir, NULL);
    ret = (RemoveDirectory(newpath) ? 0 : 1);
    LEPT_FREE(newpath);
#endif  /* !_WIN32 */

    sarrayDestroy(&sa);
    LEPT_FREE(dir);
    return ret;
}


/*!
 * \brief   lept_direxists()
 *
 * \param[in]    dir
 * \param[out]   pexists 1 if it exists; 0 otherwise
 * \return  void
 *
 * <pre>
 * Notes:
 *      (1) Always use unix pathname separators.
 *      (2) By calling genPathname(), if the pathname begins with "/tmp"
 *          this does an automatic directory translation on windows
 *          to a path in the windows <Temp> directory:
 *             "/tmp"  ==>  <Temp> (windows)
 * </pre>
 */
void
lept_direxists(const char  *dir,
               l_int32     *pexists)
{
char  *realdir;

    if (!pexists) return;
    *pexists = 0;
    if (!dir) return;
    if ((realdir = genPathname(dir, NULL)) == NULL)
        return;

#ifndef _WIN32
    {
    struct stat s;
    l_int32 err = stat(realdir, &s);
    if (err != -1 && S_ISDIR(s.st_mode))
        *pexists = 1;
    }
#else  /* _WIN32 */
    l_uint32  attributes;
    attributes = GetFileAttributes(realdir);
    if (attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        *pexists = 1;
    }
#endif  /* _WIN32 */

    LEPT_FREE(realdir);
    return;
}


/*!
 * \brief   lept_rm_match()
 *
 * \param[in]    subdir [optional]  If NULL, the removed files are in /tmp
 * \param[in]    substr [optional] pattern to match in filename
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) This removes the matched files in /tmp or a subdirectory of /tmp.
 *          Use NULL for %subdir if the files are in /tmp.
 *      (2) If %substr == NULL, this removes all files in the directory.
 *          If %substr == "" (empty), this removes no files.
 *          If both %subdir == NULL and %substr == NULL, this removes
 *          all files in /tmp.
 *      (3) Use unix pathname separators.
 *      (4) By calling genPathname(), if the pathname begins with "/tmp"
 *          this does an automatic directory translation on windows
 *          to a path in the windows <Temp> directory:
 *             "/tmp"  ==>  <Temp> (windows)
 *      (5) Error conditions:
 *            * returns -1 if the directory is not found
 *            * returns the number of files (> 0) that it was unable to remove.
 * </pre>
 */
l_int32
lept_rm_match(const char  *subdir,
              const char  *substr)
{
char    *path, *fname;
char     tempdir[256];
l_int32  i, n, ret;
SARRAY  *sa;

    PROCNAME("lept_rm_match");

    makeTempDirname(tempdir, 256, subdir);
    if ((sa = getSortedPathnamesInDirectory(tempdir, substr, 0, 0)) == NULL)
        return ERROR_INT("sa not made", procName, -1);
    n = sarrayGetCount(sa);
    if (n == 0) {
        L_WARNING("no matching files found\n", procName);
        sarrayDestroy(&sa);
        return 0;
    }

    ret = 0;
    for (i = 0; i < n; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        path = genPathname(fname, NULL);
        if (lept_rmfile(path) != 0) {
            L_ERROR("failed to remove %s\n", procName, path);
            ret++;
        }
        LEPT_FREE(path);
    }
    sarrayDestroy(&sa);
    return ret;
}


/*!
 * \brief   lept_rm()
 *
 * \param[in]    subdir [optional] of '/tmp'; can be NULL
 * \param[in]    tail filename without the directory
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) By calling genPathname(), this does an automatic directory
 *          translation on windows to a path in the windows <Temp> directory:
 *             "/tmp/..."  ==>  <Temp>/... (windows)
 * </pre>
 */
l_int32
lept_rm(const char  *subdir,
        const char  *tail)
{
char    *path;
char     newtemp[256];
l_int32  ret;

    PROCNAME("lept_rm");

    if (!tail || strlen(tail) == 0)
        return ERROR_INT("tail undefined or empty", procName, 1);

    if (makeTempDirname(newtemp, 256, subdir))
        return ERROR_INT("temp dirname not made", procName, 1);
    path = genPathname(newtemp, tail);
    ret = lept_rmfile(path);
    LEPT_FREE(path);
    return ret;
}


/*!
 * \brief
 *
 *  lept_rmfile()
 *
 * \param[in]    filepath full path to file including the directory
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) This removes the named file.
 *      (2) Use unix pathname separators.
 *      (3) There is no name translation.
 *      (4) Unlike the other lept_* functions in this section, this can remove
 *          any file -- it is not restricted to files that are in /tmp or a
 *          subdirectory of it.
 * </pre>
 */
l_int32
lept_rmfile(const char  *filepath)
{
l_int32  ret;

    PROCNAME("lept_rmfile");

    if (!filepath || strlen(filepath) == 0)
        return ERROR_INT("filepath undefined or empty", procName, 1);

#ifndef _WIN32
    ret = remove(filepath);
#else
        /* Set attributes to allow deletion of read-only files */
    SetFileAttributes(filepath, FILE_ATTRIBUTE_NORMAL);
    ret = DeleteFile(filepath) ? 0 : 1;
#endif  /* !_WIN32 */

    return ret;
}


/*!
 * \brief   lept_mv()
 *
 * \param[in]    srcfile
 * \param[in]    newdir [optional]; can be NULL
 * \param[in]    newtail [optional]; can be NULL
 * \param[out]   pnewpath [optional] of actual path; can be NULL
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) This moves %srcfile to /tmp or to a subdirectory of /tmp.
 *      (2) %srcfile can either be a full path or relative to the
 *          current directory.
 *      (3) %newdir can either specify an existing subdirectory of /tmp
 *          or can be NULL.  In the latter case, the file will be written
 *          into /tmp.
 *      (4) %newtail can either specify a filename tail or, if NULL,
 *          the filename is taken from src-tail, the tail of %srcfile.
 *      (5) For debugging, the computed newpath can be returned.  It must
 *          be freed by the caller.
 *      (6) Reminders:
 *          (a) specify files using unix pathnames
 *          (b) for windows, translates
 *                 /tmp  ==>  <Temp>
 *              where <Temp> is the windows temp directory
 *      (7) Examples:
 *          * newdir = NULL,    newtail = NULL    ==> /tmp/src-tail
 *          * newdir = NULL,    newtail = abc     ==> /tmp/abc
 *          * newdir = def/ghi, newtail = NULL    ==> /tmp/def/ghi/src-tail
 *          * newdir = def/ghi, newtail = abc     ==> /tmp/def/ghi/abc
 * </pre>
 */
l_int32
lept_mv(const char  *srcfile,
        const char  *newdir,
        const char  *newtail,
        char       **pnewpath)
{
char    *srcpath, *newpath, *realpath, *dir, *srctail;
char     newtemp[256];
l_int32  ret;

    PROCNAME("lept_mv");

    if (!srcfile)
        return ERROR_INT("srcfile not defined", procName, 1);

        /* Require output pathname to be in /tmp/ or a subdirectory */
    if (makeTempDirname(newtemp, 256, newdir) == 1)
        return ERROR_INT("newdir not NULL or a subdir of /tmp", procName, 1);

        /* Get canonical src pathname */
    splitPathAtDirectory(srcfile, &dir, &srctail);

#ifndef _WIN32
    srcpath = pathJoin(dir, srctail);
    LEPT_FREE(dir);

        /* Generate output pathname */
    if (!newtail || newtail[0] == '\0')
        newpath = pathJoin(newtemp, srctail);
    else
        newpath = pathJoin(newtemp, newtail);
    LEPT_FREE(srctail);

        /* Overwrite any existing file at 'newpath' */
    ret = fileCopy(srcpath, newpath);
    if (!ret) {
        realpath = genPathname(srcpath, NULL);
        remove(realpath);
        LEPT_FREE(realpath);
    }
#else
    srcpath = genPathname(dir, srctail);
    LEPT_FREE(dir);

        /* Generate output pathname */
    if (!newtail || newtail[0] == '\0')
        newpath = genPathname(newtemp, srctail);
    else
        newpath = genPathname(newtemp, newtail);
    LEPT_FREE(srctail);

        /* Overwrite any existing file at 'newpath' */
    ret = MoveFileEx(srcpath, newpath,
                     MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) ? 0 : 1;
#endif  /* ! _WIN32 */

    LEPT_FREE(srcpath);
    if (pnewpath)
        *pnewpath = newpath;
    else
        LEPT_FREE(newpath);
    return ret;
}


/*!
 * \brief   lept_cp()
 *
 * \param[in]    srcfile
 * \param[in]    newdir [optional]; can be NULL
 * \param[in]    newtail [optional]; can be NULL
 * \param[out]   pnewpath [optional] of actual path; can be NULL
 * \return  0 on success, non-zero on failure
 *
 * <pre>
 * Notes:
 *      (1) This copies %srcfile to /tmp or to a subdirectory of /tmp.
 *      (2) %srcfile can either be a full path or relative to the
 *          current directory.
 *      (3) %newdir can either specify an existing subdirectory of /tmp,
 *          or can be NULL.  In the latter case, the file will be written
 *          into /tmp.
 *      (4) %newtail can either specify a filename tail or, if NULL,
 *          the filename is taken from src-tail, the tail of %srcfile.
 *      (5) For debugging, the computed newpath can be returned.  It must
 *          be freed by the caller.
 *      (6) Reminders:
 *          (a) specify files using unix pathnames
 *          (b) for windows, translates
 *                 /tmp  ==>  <Temp>
 *              where <Temp> is the windows temp directory
 *      (7) Examples:
 *          * newdir = NULL,    newtail = NULL    ==> /tmp/src-tail
 *          * newdir = NULL,    newtail = abc     ==> /tmp/abc
 *          * newdir = def/ghi, newtail = NULL    ==> /tmp/def/ghi/src-tail
 *          * newdir = def/ghi, newtail = abc     ==> /tmp/def/ghi/abc
 *
 * </pre>
 */
l_int32
lept_cp(const char  *srcfile,
        const char  *newdir,
        const char  *newtail,
        char       **pnewpath)
{
char    *srcpath, *newpath, *dir, *srctail;
char     newtemp[256];
l_int32  ret;

    PROCNAME("lept_cp");

    if (!srcfile)
        return ERROR_INT("srcfile not defined", procName, 1);

        /* Require output pathname to be in /tmp or a subdirectory */
    if (makeTempDirname(newtemp, 256, newdir) == 1)
        return ERROR_INT("newdir not NULL or a subdir of /tmp", procName, 1);

       /* Get canonical src pathname */
    splitPathAtDirectory(srcfile, &dir, &srctail);

#ifndef _WIN32
    srcpath = pathJoin(dir, srctail);
    LEPT_FREE(dir);

        /* Generate output pathname */
    if (!newtail || newtail[0] == '\0')
        newpath = pathJoin(newtemp, srctail);
    else
        newpath = pathJoin(newtemp, newtail);
    LEPT_FREE(srctail);

        /* Overwrite any existing file at 'newpath' */
    ret = fileCopy(srcpath, newpath);
#else
    srcpath = genPathname(dir, srctail);
    LEPT_FREE(dir);

        /* Generate output pathname */
    if (!newtail || newtail[0] == '\0')
        newpath = genPathname(newtemp, srctail);
    else
        newpath = genPathname(newtemp, newtail);
    LEPT_FREE(srctail);

        /* Overwrite any existing file at 'newpath' */
    ret = CopyFile(srcpath, newpath, FALSE) ? 0 : 1;
#endif   /* !_WIN32 */

    LEPT_FREE(srcpath);
    if (pnewpath)
        *pnewpath = newpath;
    else
        LEPT_FREE(newpath);
    return ret;
}


/*--------------------------------------------------------------------*
 *                     General file name operations                   *
 *--------------------------------------------------------------------*/
/*!
 * \brief   splitPathAtDirectory()
 *
 * \param[in]    pathname  full path; can be a directory
 * \param[out]   pdir  [optional] root directory name of
 *                     input path, including trailing '/'
 * \param[out]   ptail [optional] path tail, which is either
 *                     the file name within the root directory or
 *                     the last sub-directory in the path
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) If you only want the tail, input null for the root directory ptr.
 *      (2) If you only want the root directory name, input null for the
 *          tail ptr.
 *      (3) This function makes decisions based only on the lexical
 *          structure of the input.  Examples:
 *            /usr/tmp/abc  -->  dir: /usr/tmp/       tail: abc
 *            /usr/tmp/     -->  dir: /usr/tmp/       tail: [empty string]
 *            /usr/tmp      -->  dir: /usr/           tail: tmp
 *            abc           -->  dir: [empty string]  tail: abc
 *      (4) The input can have either forward (unix) or backward (win)
 *          slash separators.  The output has unix separators.
 *          Note that Win32 pathname functions generally accept both
 *          slash forms, but the windows command line interpreter
 *          only accepts backward slashes, because forward slashes are
 *          used to demarcate switches (vs. dashes in unix).
 * </pre>
 */
l_int32
splitPathAtDirectory(const char  *pathname,
                     char       **pdir,
                     char       **ptail)
{
char  *cpathname, *lastslash;

    PROCNAME("splitPathAtDirectory");

    if (!pdir && !ptail)
        return ERROR_INT("null input for both strings", procName, 1);
    if (pdir) *pdir = NULL;
    if (ptail) *ptail = NULL;
    if (!pathname)
        return ERROR_INT("pathname not defined", procName, 1);

    cpathname = stringNew(pathname);
    convertSepCharsInPath(cpathname, UNIX_PATH_SEPCHAR);
    lastslash = strrchr(cpathname, '/');
    if (lastslash) {
        if (ptail)
            *ptail = stringNew(lastslash + 1);
        if (pdir) {
            *(lastslash + 1) = '\0';
            *pdir = cpathname;
        } else {
            LEPT_FREE(cpathname);
        }
    } else {  /* no directory */
        if (pdir)
            *pdir = stringNew("");
        if (ptail)
            *ptail = cpathname;
        else
            LEPT_FREE(cpathname);
    }

    return 0;
}


/*!
 * \brief   splitPathAtExtension()
 *
 * \param[in]    pathname full path; can be a directory
 * \param[out]   pbasename [optional] pathname not including the
 *                        last dot and characters after that
 * \param[out]   pextension [optional] path extension, which is
 *                        the last dot and the characters after it.  If
 *                        there is no extension, it returns the empty string
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) If you only want the extension, input null for the basename ptr.
 *      (2) If you only want the basename without extension, input null
 *          for the extension ptr.
 *      (3) This function makes decisions based only on the lexical
 *          structure of the input.  Examples:
 *            /usr/tmp/abc.jpg  -->  basename: /usr/tmp/abc    ext: .jpg
 *            /usr/tmp/.jpg     -->  basename: /usr/tmp/       ext: .jpg
 *            /usr/tmp.jpg/     -->  basename: /usr/tmp.jpg/   ext: [empty str]
 *            ./.jpg            -->  basename: ./              ext: .jpg
 *      (4) The input can have either forward (unix) or backward (win)
 *          slash separators.  The output has unix separators.
 * </pre>
 */
l_int32
splitPathAtExtension(const char  *pathname,
                     char       **pbasename,
                     char       **pextension)
{
char  *tail, *dir, *lastdot;
char   empty[4] = "";

    PROCNAME("splitPathExtension");

    if (!pbasename && !pextension)
        return ERROR_INT("null input for both strings", procName, 1);
    if (pbasename) *pbasename = NULL;
    if (pextension) *pextension = NULL;
    if (!pathname)
        return ERROR_INT("pathname not defined", procName, 1);

        /* Split out the directory first */
    splitPathAtDirectory(pathname, &dir, &tail);

        /* Then look for a "." in the tail part.
         * This way we ignore all "." in the directory. */
    if ((lastdot = strrchr(tail, '.'))) {
        if (pextension)
            *pextension = stringNew(lastdot);
        if (pbasename) {
            *lastdot = '\0';
            *pbasename = stringJoin(dir, tail);
        }
    } else {
        if (pextension)
            *pextension = stringNew(empty);
        if (pbasename)
            *pbasename = stringNew(pathname);
    }
    LEPT_FREE(dir);
    LEPT_FREE(tail);
    return 0;
}


/*!
 * \brief   pathJoin()
 *
 * \param[in]    dir [optional] can be null
 * \param[in]    fname [optional] can be null
 * \return  specially concatenated path, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) Use unix-style pathname separators ('/').
 *      (2) %fname can be the entire path, or part of the path containing
 *          at least one directory, or a tail without a directory, or NULL.
 *      (3) It produces a path that strips multiple slashes to a single
 *          slash, joins %dir and %fname by a slash, and has no trailing
 *          slashes (except in the cases where %dir == "/" and
 *          %fname == NULL, or v.v.).
 *      (4) If both %dir and %fname are null, produces an empty string.
 *      (5) Neither %dir nor %fname can begin with '..'.
 *      (6) The result is not canonicalized or tested for correctness:
 *          garbage in (e.g., /&%), garbage out.
 *      (7) Examples:
 *             //tmp// + //abc/  -->  /tmp/abc
 *             tmp/ + /abc/      -->  tmp/abc
 *             tmp/ + abc/       -->  tmp/abc
 *             /tmp/ + ///       -->  /tmp
 *             /tmp/ + NULL      -->  /tmp
 *             // + /abc//       -->  /abc
 *             // + NULL         -->  /
 *             NULL + /abc/def/  -->  /abc/def
 *             NULL + abc//      -->  abc
 *             NULL + //         -->  /
 *             NULL + NULL       -->  (empty string)
 *             "" + ""           -->  (empty string)
 *             "" + /            -->  /
 *             ".." + /etc/foo   -->  NULL
 *             /tmp + ".."       -->  NULL
 * </pre>
 */
char *
pathJoin(const char  *dir,
         const char  *fname)
{
char     *slash = (char *)"/";
char     *str, *dest;
l_int32   i, n1, n2, emptydir;
size_t    size;
SARRAY   *sa1, *sa2;
L_BYTEA  *ba;

    PROCNAME("pathJoin");

    if (!dir && !fname)
        return stringNew("");
    if (dir && strlen(dir) >= 2 && dir[0] == '.' && dir[1] == '.')
        return (char *)ERROR_PTR("dir starts with '..'", procName, NULL);
    if (fname && strlen(fname) >= 2 && fname[0] == '.' && fname[1] == '.')
        return (char *)ERROR_PTR("fname starts with '..'", procName, NULL);

    sa1 = sarrayCreate(0);
    sa2 = sarrayCreate(0);
    ba = l_byteaCreate(4);

        /* Process %dir */
    if (dir && strlen(dir) > 0) {
        if (dir[0] == '/')
            l_byteaAppendString(ba, slash);
        sarraySplitString(sa1, dir, "/");  /* removes all slashes */
        n1 = sarrayGetCount(sa1);
        for (i = 0; i < n1; i++) {
            str = sarrayGetString(sa1, i, L_NOCOPY);
            l_byteaAppendString(ba, str);
            l_byteaAppendString(ba, slash);
        }
    }

        /* Special case to add leading slash: dir NULL or empty string  */
    emptydir = dir && strlen(dir) == 0;
    if ((!dir || emptydir) && fname && strlen(fname) > 0 && fname[0] == '/')
        l_byteaAppendString(ba, slash);

        /* Process %fname */
    if (fname && strlen(fname) > 0) {
        sarraySplitString(sa2, fname, "/");
        n2 = sarrayGetCount(sa2);
        for (i = 0; i < n2; i++) {
            str = sarrayGetString(sa2, i, L_NOCOPY);
            l_byteaAppendString(ba, str);
            l_byteaAppendString(ba, slash);
        }
    }

        /* Remove trailing slash */
    dest = (char *)l_byteaCopyData(ba, &size);
    if (size > 1 && dest[size - 1] == '/')
        dest[size - 1] = '\0';

    sarrayDestroy(&sa1);
    sarrayDestroy(&sa2);
    l_byteaDestroy(&ba);
    return dest;
}


/*!
 * \brief   appendSubdirs()
 *
 * \param[in]    basedir
 * \param[in]    subdirs
 * \return  concatenated full directory path without trailing slash,
 *              or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) Use unix pathname separators
 *      (2) Allocates a new string:  <basedir>/<subdirs>
 * </pre>
 */
char *
appendSubdirs(const char  *basedir,
              const char  *subdirs)
{
char   *newdir;
size_t  len1, len2, len3, len4;

    PROCNAME("appendSubdirs");

    if (!basedir || !subdirs)
        return (char *)ERROR_PTR("basedir and subdirs not both defined",
                                 procName, NULL);

    len1 = strlen(basedir);
    len2 = strlen(subdirs);
    len3 = len1 + len2 + 6;
    if ((newdir = (char *)LEPT_CALLOC(len3 + 1, 1)) == NULL)
        return (char *)ERROR_PTR("newdir not made", procName, NULL);
    strncat(newdir, basedir, len3);  /* add basedir */
    if (newdir[len1 - 1] != '/')  /* add '/' if necessary */
        newdir[len1] = '/';
    if (subdirs[0] == '/')  /* add subdirs, stripping leading '/' */
        strncat(newdir, subdirs + 1, len3);
    else
        strncat(newdir, subdirs, len3);
    len4 = strlen(newdir);
    if (newdir[len4 - 1] == '/')  /* strip trailing '/' */
        newdir[len4 - 1] = '\0';

    return newdir;
}


/*--------------------------------------------------------------------*
 *                     Special file name operations                   *
 *--------------------------------------------------------------------*/
/*!
 * \brief   convertSepCharsInPath()
 *
 * \param[in]    path
 * \param[in]    type UNIX_PATH_SEPCHAR, WIN_PATH_SEPCHAR
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) In-place conversion.
 *      (2) Type is the resulting type:
 *            * UNIX_PATH_SEPCHAR:  '\\' ==> '/'
 *            * WIN_PATH_SEPCHAR:   '/' ==> '\\'
 *      (3) Virtually all path operations in leptonica use unix separators.
 * </pre>
 */
l_int32
convertSepCharsInPath(char    *path,
                      l_int32  type)
{
l_int32  i;
size_t   len;

    PROCNAME("convertSepCharsInPath");
    if (!path)
        return ERROR_INT("path not defined", procName, 1);
    if (type != UNIX_PATH_SEPCHAR && type != WIN_PATH_SEPCHAR)
        return ERROR_INT("invalid type", procName, 1);

    len = strlen(path);
    if (type == UNIX_PATH_SEPCHAR) {
        for (i = 0; i < len; i++) {
            if (path[i] == '\\')
                path[i] = '/';
        }
    } else {  /* WIN_PATH_SEPCHAR */
        for (i = 0; i < len; i++) {
            if (path[i] == '/')
                path[i] = '\\';
        }
    }
    return 0;
}


/*!
 * \brief   genPathname()
 *
 * \param[in]    dir [optional] directory or full path name, with or without
 *                   trailing '/'
 * \param[in]    fname [optional] file name within a directory
 * \return  pathname either a directory or full path, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) This function generates actual paths in the following ways:
 *            * from two sub-parts (e.g., a directory and a file name).
 *            * from a single path full path, placed in %dir, with
 *              %fname == NULL.
 *            * from the name of a file in the local directory placed in
 *              %fname, with %dir == NULL.
 *            * if in a "/tmp" directory and on windows, the windows
 *              temp directory is used.
 *      (2) On windows, if the root of %dir is '/tmp', this does a name
 *          translation:
 *             "/tmp"  ==>  <Temp> (windows)
 *          where <Temp> is the windows temp directory.
 *      (3) On unix, the TMPDIR variable is ignored.  No rewriting
 *          of temp directories is permitted.
 *      (4) There are four cases for the input:
 *          (a) %dir is a directory and %fname is defined: result is a full path
 *          (b) %dir is a directory and %fname is null: result is a directory
 *          (c) %dir is a full path and %fname is null: result is a full path
 *          (d) %dir is null or an empty string: start in the current dir;
 *              result is a full path
 *      (5) In all cases, the resulting pathname is not terminated with a slash
 *      (6) The caller is responsible for freeing the returned pathname.
 * </pre>
 */
char *
genPathname(const char  *dir,
            const char  *fname)
{
l_int32  is_win32 = FALSE;
char    *cdir, *pathout;
l_int32  dirlen, namelen, size;

    PROCNAME("genPathname");

    if (!dir && !fname)
        return (char *)ERROR_PTR("no input", procName, NULL);

        /* Handle the case where we start from the current directory */
    if (!dir || dir[0] == '\0') {
        if ((cdir = getcwd(NULL, 0)) == NULL)
            return (char *)ERROR_PTR("no current dir found", procName, NULL);
    } else {
        cdir = stringNew(dir);
    }

        /* Convert to unix path separators, and remove the trailing
         * slash in the directory, except when dir == "/"  */
    convertSepCharsInPath(cdir, UNIX_PATH_SEPCHAR);
    dirlen = strlen(cdir);
    if (cdir[dirlen - 1] == '/' && dirlen != 1) {
        cdir[dirlen - 1] = '\0';
        dirlen--;
    }

    namelen = (fname) ? strlen(fname) : 0;
    size = dirlen + namelen + 256;
    if ((pathout = (char *)LEPT_CALLOC(size, sizeof(char))) == NULL) {
        LEPT_FREE(cdir);
        return (char *)ERROR_PTR("pathout not made", procName, NULL);
    }

#ifdef _WIN32
    is_win32 = TRUE;
#endif  /* _WIN32 */

        /* First handle %dir (which may be a full pathname).
         * There is no path rewriting on unix, and on win32, we do not
         * rewrite unless the specified directory is /tmp or
         * a subdirectory of /tmp */
    if (!is_win32 || dirlen < 4 ||
        (dirlen == 4 && strncmp(cdir, "/tmp", 4) != 0) ||  /* not in "/tmp" */
        (dirlen > 4 && strncmp(cdir, "/tmp/", 5) != 0)) {  /* not in "/tmp/" */
        stringCopy(pathout, cdir, dirlen);
    } else {  /* Rewrite for win32 with "/tmp" specified for the directory. */
#ifdef _WIN32
        l_int32 tmpdirlen;
        char tmpdir[MAX_PATH];
        GetTempPath(sizeof(tmpdir), tmpdir);  /* get the windows temp dir */
        tmpdirlen = strlen(tmpdir);
        if (tmpdirlen > 0 && tmpdir[tmpdirlen - 1] == '\\') {
            tmpdir[tmpdirlen - 1] = '\0';  /* trim the trailing '\' */
        }
        tmpdirlen = strlen(tmpdir);
        stringCopy(pathout, tmpdir, tmpdirlen);

            /* Add the rest of cdir */
        if (dirlen > 4)
            stringCat(pathout, size, cdir + 4);
#endif  /* _WIN32 */
    }

        /* Now handle %fname */
    if (fname && strlen(fname) > 0) {
        dirlen = strlen(pathout);
        pathout[dirlen] = '/';
        strncat(pathout, fname, namelen);
    }

    LEPT_FREE(cdir);
    return pathout;
}


/*!
 * \brief   makeTempDirname()
 *
 * \param[in]    result preallocated on stack or heap and passed in
 * \param[in]    nbytes size of %result array, in bytes
 * \param[in]    subdir [optional]; can be NULL or an empty string
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This generates the directory path for output temp files,
 *          written into %result with unix separators.
 *      (2) Caller allocates %result, large enough to hold the path,
 *          which is:
 *            /tmp/%subdir       (unix)
 *            <Temp>/%subdir     (windows)
 *          where <Temp> is a path on windows determined by GenTempPath()
 *          and %subdir is in general a set of nested subdirectories:
 *            dir1/dir2/.../dirN
 *          which in use would not typically exceed 2 levels.
 *      (3) Usage example:
 * \code
 *           char  result[256];
 *           makeTempDirname(result, 256, "lept/golden");
 * \endcode
 * </pre>
 */
l_int32
makeTempDirname(char        *result,
                size_t       nbytes,
                const char  *subdir)
{
char    *dir, *path;
l_int32  ret = 0;
size_t   pathlen;

    PROCNAME("makeTempDirname");

    if (!result)
        return ERROR_INT("result not defined", procName, 1);
    if (subdir && ((subdir[0] == '.') || (subdir[0] == '/')))
        return ERROR_INT("subdir not an actual subdirectory", procName, 1);

    memset(result, 0, nbytes);
    dir = pathJoin("/tmp", subdir);
#ifndef _WIN32
    path = stringNew(dir);
#else
    path = genPathname(dir, NULL);
#endif  /*  ~ _WIN32 */
    pathlen = strlen(path);
    if (pathlen < nbytes - 1) {
        strncpy(result, path, pathlen);
    } else {
        L_ERROR("result array too small for path\n", procName);
        ret = 1;
    }

    LEPT_FREE(dir);
    LEPT_FREE(path);
    return ret;
}


/*!
 * \brief   modifyTrailingSlash()
 *
 * \param[in]    path preallocated on stack or heap and passed in
 * \param[in]    nbytes size of %path array, in bytes
 * \param[in]    flag L_ADD_TRAIL_SLASH or L_REMOVE_TRAIL_SLASH
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This carries out the requested action if necessary.
 * </pre>
 */
l_int32
modifyTrailingSlash(char    *path,
                    size_t   nbytes,
                    l_int32  flag)
{
char    lastchar;
size_t  len;

    PROCNAME("modifyTrailingSlash");

    if (!path)
        return ERROR_INT("path not defined", procName, 1);
    if (flag != L_ADD_TRAIL_SLASH && flag != L_REMOVE_TRAIL_SLASH)
        return ERROR_INT("invalid flag", procName, 1);

    len = strlen(path);
    lastchar = path[len - 1];
    if (flag == L_ADD_TRAIL_SLASH && lastchar != '/' && len < nbytes - 2) {
        path[len] = '/';
        path[len + 1] = '\0';
    } else if (flag == L_REMOVE_TRAIL_SLASH && lastchar == '/') {
        path[len - 1] = '\0';
    }
    return 0;
}


/*!
 * \brief   l_makeTempFilename()
 *
 * \return  fname : heap allocated filename; returns NULL on failure.
 *
 * <pre>
 * Notes:
 *      (1) On unix, this makes a filename of the form
 *               "/tmp/lept.XXXXXX",
 *          where each X is a random character.
 *      (2) On windows, this makes a filename of the form
 *               "/<Temp>/lp.XXXXXX".
 *      (3) On all systems, this fails if the file is not writable.
 *      (4) Safest usage is to write to a subdirectory in debug code.
 *      (5) The returned filename must be freed by the caller, using lept_free.
 *      (6) The tail of the filename has a '.', so that cygwin interprets
 *          the file as having an extension.  Otherwise, cygwin assumes it
 *          is an executable and appends ".exe" to the filename.
 *      (7) On unix, whenever possible use tmpfile() instead.  tmpfile()
 *          hides the file name, returns a stream opened for write,
 *          and deletes the temp file when the stream is closed.
 */
char *
l_makeTempFilename()
{
char  dirname[240];

    PROCNAME("l_makeTempFilename");

    if (makeTempDirname(dirname, sizeof(dirname), NULL) == 1)
        return (char *)ERROR_PTR("failed to make dirname", procName, NULL);

#ifndef _WIN32
{
    char    *pattern;
    l_int32  fd;
    pattern = stringConcatNew(dirname, "/lept.XXXXXX", NULL);
    fd = mkstemp(pattern);
    if (fd == -1) {
        LEPT_FREE(pattern);
        return (char *)ERROR_PTR("mkstemp failed", procName, NULL);
    }
    close(fd);
    return pattern;
}
#else
{
    char  fname[MAX_PATH];
    FILE *fp;
    if (GetTempFileName(dirname, "lp.", 0, fname) == 0)
        return (char *)ERROR_PTR("GetTempFileName failed", procName, NULL);
    if ((fp = fopen(fname, "wb")) == NULL)
        return (char *)ERROR_PTR("file cannot be written to", procName, NULL);
    fclose(fp);
    return stringNew(fname);
}
#endif  /*  ~ _WIN32 */
}


/*!
 * \brief   extractNumberFromFilename()
 *
 * \param[in]    fname
 * \param[in]    numpre number of characters before the digits to be found
 * \param[in]    numpost number of characters after the digits to be found
 * \return  num number embedded in the filename; -1 on error or if
 *                   not found
 *
 * <pre>
 * Notes:
 *      (1) The number is to be found in the basename, which is the
 *          filename without either the directory or the last extension.
 *      (2) When a number is found, it is non-negative.  If no number
 *          is found, this returns -1, without an error message.  The
 *          caller needs to check.
 * </pre>
 */
l_int32
extractNumberFromFilename(const char  *fname,
                          l_int32      numpre,
                          l_int32      numpost)
{
char    *tail, *basename;
l_int32  len, nret, num;

    PROCNAME("extractNumberFromFilename");

    if (!fname)
        return ERROR_INT("fname not defined", procName, -1);

    splitPathAtDirectory(fname, NULL, &tail);
    splitPathAtExtension(tail, &basename, NULL);
    LEPT_FREE(tail);

    len = strlen(basename);
    if (numpre + numpost > len - 1) {
        LEPT_FREE(basename);
        return ERROR_INT("numpre + numpost too big", procName, -1);
    }

    basename[len - numpost] = '\0';
    nret = sscanf(basename + numpre, "%d", &num);
    LEPT_FREE(basename);

    if (nret == 1)
        return num;
    else
        return -1;  /* not found */
}
