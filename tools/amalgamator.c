/*
This tool uses c89str itself which means this program must work without depending on any
amalgamated code.

Useage: amalgamator [path to c89str.h] [path to stb_sprintf.h]
*/
#define C89STR_IMPLEMENTATION
#include "../c89str.h"

FILE* c89str_fopen(const char* pFilePath, const char* pOpenMode)
{
    FILE* pFile;

#if defined(_MSC_VER)
    errno_t result = fopen_s(&pFile, pFilePath, pOpenMode);
    if (result != 0) {
        return NULL;
    }
#else
    pFile = fopen(pFilePath, pOpenMode);
#endif

    return pFile;
}

c89str c89str_open_and_read_text_file(const char* pFilePath)
{
    c89str str;
    FILE* pFile;
    size_t fileSize;
    char* pFileContents;

    if (pFilePath == NULL) {
        return NULL;
    }

    pFile = c89str_fopen(pFilePath, "rb");
    if (pFile == NULL) {
        printf("Failed to open file \"%s\"", pFilePath);
        return NULL;
    }

    fseek(pFile, 0, SEEK_END);
    fileSize = (size_t)ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    pFileContents = c89str_malloc(fileSize, NULL);
    if (pFileContents == NULL) {
        return NULL;    /* Out of memory. */
    }

    fread(pFileContents, 1, fileSize, pFile);
    fclose(pFile);

    str = c89str_newn(NULL, pFileContents, fileSize);
    
    c89str_free(pFileContents, NULL);

    return str;
}

errno_t c89str_open_and_write_text_file(const char* pFilePath, const char* pFileContent)
{
    FILE* pFile;
    size_t len;
    
    pFile = c89str_fopen(pFilePath, "wb");
    if (pFile == NULL) {
        return ENOENT;  /* Failed to open file. */
    }

    len = c89str_strlen(pFileContent);
    fwrite(pFileContent, 1, len, pFile);
    fclose(pFile);

    return C89STR_SUCCESS;
}




C89STR_API const char* c89str_substr_tagged(const char* str, const char* pTagBeg, const char* pTagEnd, size_t* pLen)
{
    size_t offsetBeg;
    size_t offsetEnd;
    errno_t result;

    if (pLen != NULL) {
        *pLen = 0;
    }

    if (pTagBeg == NULL || pTagBeg[0] == '\0') {
        offsetBeg = 0;
    } else {
        result = c89str_find(str, pTagBeg, &offsetBeg);
        if (result != C89STR_SUCCESS) {
            return NULL; /* Could not find the begin tag in the other string. */
        }
    }

    if (pTagEnd == NULL || pTagEnd[0] == '\0') {
        offsetEnd = c89str_strlen(str);
    } else {
        result = c89str_find(str + offsetBeg + c89str_strlen(pTagBeg), pTagEnd, &offsetEnd);
        if (result != C89STR_SUCCESS) {
            return NULL; /* Could not find the end tag in the other string. */
        } else {
            offsetEnd += offsetBeg + c89str_strlen(pTagBeg) + c89str_strlen(pTagEnd);
        }
    }

    if (pLen != NULL) {
        *pLen = (offsetEnd - offsetBeg);
    }

    return str + offsetBeg;
}

C89STR_API errno_t c89str_new_substr_tagged(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, const char* pTagBeg, const char* pTagEnd)
{
    size_t len;
    pOther = c89str_substr_tagged(pOther, pTagBeg, pTagEnd, &len);
    if (pOther == NULL) {
        return ENOENT;
    }

    *pStr = c89str_newn(pAllocationCallbacks, pOther, len);
    return c89str_result(*pStr);
}

C89STR_API errno_t c89str_replace_range_tagged(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pTagBeg, const char* pTagEnd, const char* pOther, const char* pOtherTagBeg, const char* pOtherTagEnd, c89str_bool32 keepTagsOnSeparateLines)
{
    errno_t result;
    size_t strOffsetBeg;
    size_t strOffsetEnd;
    size_t otherSubstrLen;
    const char* pOtherSubstr;
    const char* pOtherNewLines = NULL;

    if (pStr == NULL) {
        return EINVAL;
    }

    if (*pStr == NULL || pOther == NULL) {
        return EINVAL;
    }

    if (pTagBeg == NULL || pTagBeg[0] == '\0') {
        strOffsetBeg = 0;
    } else {
        result = c89str_find(*pStr, pTagBeg, &strOffsetBeg);
        if (result != C89STR_SUCCESS) {
            return result; /* Could not find begin tag. */
        } else {
            strOffsetBeg += c89str_strlen(pTagBeg);   /* Don't want to replace the tag itself. */
        }
    }

    if (pTagEnd == NULL || pTagEnd[0] == '\0') {
        strOffsetEnd = c89str_get_len(*pStr);
    } else {
        result = c89str_find(*pStr + strOffsetBeg, pTagEnd, &strOffsetEnd);
        if (result != C89STR_SUCCESS) {
            return result; /* Could not find end tag. */
        } else {
            strOffsetEnd += strOffsetBeg;   /* When we searched for the string we started from the end of the beginning tag. Need to normalize the end offset. */
        }
    }

    pOtherSubstr = c89str_substr_tagged(pOther, pOtherTagBeg, pOtherTagEnd, &otherSubstrLen);
    if (pOtherSubstr == NULL) {
        return ENOENT;  /* Failed to retrieve the substring of the other string. */
    }

    if (keepTagsOnSeparateLines) {
        pOtherNewLines = "\n";
    }

    *pStr = c89str_replace_ex(*pStr, pAllocationCallbacks, strOffsetBeg, strOffsetEnd - strOffsetBeg, pOtherSubstr, otherSubstrLen, pOtherNewLines, pOtherNewLines);
    return c89str_result(*pStr);
}



void replace_stbsp_namespaces(c89str* pStr)
{
    *pStr = c89str_replace_all(*pStr, NULL, "STBSP__", (size_t)-1, "C89STR_", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "STBSP_",  (size_t)-1, "C89STR_", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "STB_",    (size_t)-1, "C89STR_", (size_t)-1);

    *pStr = c89str_replace_all(*pStr, NULL, "stbsp__", (size_t)-1, "c89str_", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "stbsp_",  (size_t)-1, "c89str_", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "stb_",    (size_t)-1, "c89str_", (size_t)-1);
}

void style_cleanup(c89str* pStr)
{
    *pStr = c89str_replace_all(*pStr, NULL, "void *", (size_t)-1, "void* ", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "char *", (size_t)-1, "char* ", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "char const *", (size_t)-1, "char const* ", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "C89STR_SPRINTFCB *", (size_t)-1, "C89STR_SPRINTFCB* ", (size_t)-1);

    *pStr = c89str_replace_all(*pStr, NULL, "C89STR_ATTRIBUTE_FORMAT(2,3)", (size_t)-1, "C89STR_ATTRIBUTE_FORMAT(2, 3)", (size_t)-1);
    *pStr = c89str_replace_all(*pStr, NULL, "C89STR_ATTRIBUTE_FORMAT(3,4)", (size_t)-1, "C89STR_ATTRIBUTE_FORMAT(3, 4)", (size_t)-1);

    *pStr = c89str_replace_all(*pStr, NULL, "C89STR_SPRINTFCB", (size_t)-1, "c89str_sprintf_callback", (size_t)-1);
}

void remove_comments(c89str* pStr)
{
    c89str newStr = c89str_new(NULL, NULL);

    /* To remove comments I'm just going to use the lexer. The lexer provides us enough information that we can recreate the code. */
    c89str_lexer lexer;
    c89str_lexer_init(&lexer, *pStr, (size_t)-1);
    while (c89str_lexer_next(&lexer) == C89STR_SUCCESS) {
        if (lexer.token == c89str_token_type_comment) {
            continue;   /* Skip over comments. */
        }

        newStr = c89str_catn(newStr, NULL, lexer.pTokenStr, lexer.tokenLen);
    }

    *pStr = newStr;
}




int main(int argc, char** argv)
{
    errno_t result;
    c89str c89strFileContent;
    c89str stbFileContent;

    if (argc < 3) {
        printf("No input files. Specify the path to c89str.h and stb_sprintf.h in that order: amalgamator [c89str.h] [stb_sprintf.h]");
        return -1;
    }

    c89strFileContent = c89str_open_and_read_text_file(argv[1]);
    if (c89strFileContent == NULL) {
        printf("Could not open c89str.h");
        return -1;
    }

    stbFileContent = c89str_open_and_read_text_file(argv[2]);
    if (stbFileContent == NULL) {
        printf("Could not open stb_sprintf.h");
        return -1;
    }


    /*
    We have the necessary data we need to do our amalgamation. The first thing to do is isolate the header and implementation
    sections from stb_sprintf.h. This should be easy enough since they'll be wrapped in pre-processor guards.
    */
    c89str stbHeadSection = NULL;
    c89str stbImplSection = NULL;
    const char* pSTBHeadTagOpening = "#ifndef STB_SPRINTF_H_INCLUDE";
    const char* pSTBHeadTagClosing = "#endif // STB_SPRINTF_H_INCLUDE";
    const char* pSTBImplTagOpening = "#ifdef STB_SPRINTF_IMPLEMENTATION";
    const char* pSTBImplTagClosing = "#endif // STB_SPRINTF_IMPLEMENTATION";

    result = c89str_new_substr_tagged(&stbHeadSection, NULL, stbFileContent, pSTBHeadTagOpening, pSTBHeadTagClosing);
    if (result != C89STR_SUCCESS) {
        printf("Could not find header section in stb_sprintf.h");
        return -1;
    }

    result = c89str_new_substr_tagged(&stbImplSection, NULL, stbFileContent, pSTBImplTagOpening, pSTBImplTagClosing);
    if (result != C89STR_SUCCESS) {
        printf("Could not find implementation section in stb_sprintf.h");
        return -1;
    }

    /*
    At this point we have split the stb_sprintf() file and now we need to do some cleanup:

        * The `#ifndef *` and `#endif` should be removed since they're not needed (they'll be guarded with c89str's guards).
        * Comments can be stripped out since they're not readable, but also because it uses line comments which break some supported targets of c89str.
        * Everything needs to be namespaced with "c89str_".
    */
    c89str stbHeadSectionClean = c89str_new(NULL, stbHeadSection);
    c89str stbImplSectionClean = c89str_new(NULL, stbImplSection);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "#ifndef STB_SPRINTF_H_INCLUDE",   (size_t)-1, "", 0);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "#define STB_SPRINTF_H_INCLUDE",   (size_t)-1, "", 0);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "#endif // STB_SPRINTF_H_INCLUDE", (size_t)-1, "", 0);

    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "#ifdef STB_SPRINTF_IMPLEMENTATION",    (size_t)-1, "", 0);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "#endif // STB_SPRINTF_IMPLEMENTATION", (size_t)-1, "", 0);

    /*
    A big chunk of the header section can be removed because we have the equivalent in c89str. It's the part
    that begins on the line with "#ifdef STB_SPRINTF_STATIC" and goes until the start (not including) the
    line that looks like "#ifndef STB_SPRINTF_MIN".

    The only thing we'll need to add is a replacement for STBSP__PUBLICDEF so that it incorporates STBSP__ASAN.
    This entire thing can be achieved with a replace operation.
    */
    size_t blockBeg;
    size_t blockEnd;
    const char* pReplacement;

    result = c89str_find(stbHeadSectionClean, "#ifdef STB_SPRINTF_STATIC", &blockBeg);
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h header section.");
        return -1;
    }
    result = c89str_find(stbHeadSectionClean, "#ifndef STB_SPRINTF_MIN", &blockEnd);
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h header section.");
        return -1;
    }

    pReplacement =
        "#ifndef C89STR_API_SPRINTF_DEF\n"
        "#define C89STR_API_SPRINTF_DEF C89STR_API C89STR_ASAN\n"
        "#endif\n\n";

    stbHeadSectionClean = c89str_replace(stbHeadSectionClean, NULL, blockBeg, blockEnd - blockBeg, pReplacement, (size_t)-1);



    /*
    We don't care about the STB_SPRINTF_DECORATE part either. We'll hard code our names.
    */
    result = c89str_find(stbHeadSectionClean, "#ifndef STB_SPRINTF_DECORATE", &blockBeg);
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h header section.");
        return -1;
    }
    result = c89str_find(stbHeadSectionClean, "STBSP__PUBLICDEC", &blockEnd);    /* <-- Will not be included in the replaced code. */
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h header section.");
        return -1;
    }

    stbHeadSectionClean = c89str_replace(stbHeadSectionClean, NULL, blockBeg, blockEnd - blockBeg, "", (size_t)-1);



    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STBSP__PUBLICDEC", (size_t)-1, "C89STR_API", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STBSP__PUBLICDEC", (size_t)-1, "C89STR_API", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STBSP__PUBLICDEF", (size_t)-1, "C89STR_API_SPRINTF_DEF", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STBSP__PUBLICDEF", (size_t)-1, "C89STR_API_SPRINTF_DEF", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STBSP__ATTRIBUTE_FORMAT", (size_t)-1, "C89STR_ATTRIBUTE_FORMAT", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STBSP__ATTRIBUTE_FORMAT", (size_t)-1, "C89STR_ATTRIBUTE_FORMAT", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STBSP__NOTUSED", (size_t)-1, "C89STR_UNUSED", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STBSP__NOTUSED", (size_t)-1, "C89STR_UNUSED", (size_t)-1);


    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE(vsprintf)", (size_t)-1, "c89str_vsprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE(vsprintf)", (size_t)-1, "c89str_vsprintf", (size_t)-1);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE( vsprintf )", (size_t)-1, "c89str_vsprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE( vsprintf )", (size_t)-1, "c89str_vsprintf", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE(vsnprintf)", (size_t)-1, "c89str_vsnprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE(vsnprintf)", (size_t)-1, "c89str_vsnprintf", (size_t)-1);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE( vsnprintf )", (size_t)-1, "c89str_vsnprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE( vsnprintf )", (size_t)-1, "c89str_vsnprintf", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE(sprintf)", (size_t)-1, "c89str_sprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE(sprintf)", (size_t)-1, "c89str_sprintf", (size_t)-1);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE( sprintf )", (size_t)-1, "c89str_sprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE( sprintf )", (size_t)-1, "c89str_sprintf", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE(snprintf)", (size_t)-1, "c89str_snprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE(snprintf)", (size_t)-1, "c89str_snprintf", (size_t)-1);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE( snprintf )", (size_t)-1, "c89str_snprintf", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE( snprintf )", (size_t)-1, "c89str_snprintf", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE(vsprintfcb)", (size_t)-1, "c89str_vsprintfcb", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE(vsprintfcb)", (size_t)-1, "c89str_vsprintfcb", (size_t)-1);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE( vsprintfcb )", (size_t)-1, "c89str_vsprintfcb", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE( vsprintfcb )", (size_t)-1, "c89str_vsprintfcb", (size_t)-1);

    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE(set_separators)", (size_t)-1, "c89str_set_sprintf_separators", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE(set_separators)", (size_t)-1, "c89str_set_sprintf_separators", (size_t)-1);
    stbHeadSectionClean = c89str_replace_all(stbHeadSectionClean, NULL, "STB_SPRINTF_DECORATE( set_separators )", (size_t)-1, "c89str_set_sprintf_separators", (size_t)-1);
    stbImplSectionClean = c89str_replace_all(stbImplSectionClean, NULL, "STB_SPRINTF_DECORATE( set_separators )", (size_t)-1, "c89str_set_sprintf_separators", (size_t)-1);
    


    /*
    There's a section in the header that can be moved to the implementation section. At the same time, there's a number
    of sized types that are defined in the implementation section that need not exist.
    */
    result = c89str_find(stbHeadSectionClean, "#if defined(__clang__)", &blockBeg);
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h header section.");
        return -1;
    }
    result = c89str_find(stbHeadSectionClean, "typedef char *STBSP_SPRINTFCB", &blockEnd);    /* <-- Will not be included in the replaced code. */
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h header section.");
        return -1;
    }

    c89str sectionToMoveFromHeadToImpl = c89str_newn(NULL, stbHeadSectionClean + blockBeg, blockEnd - blockBeg);
    

    /* We have a copy of the text in preparation for moving. We can now delete it from the header. */
    stbHeadSectionClean = c89str_remove(stbHeadSectionClean, NULL, blockBeg, blockEnd);

    /*
    Now we need to insert this into the top of the implementation section. Already at the top of the implementation
    is a section that contains some declarations of some sized types which we want to get rid of anyway. We'll just
    replace that section with the part from the header.
    */
    result = c89str_find(stbImplSectionClean, "#define stbsp__uint32 unsigned int", &blockBeg);
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h implementation section.");
        return -1;
    }
    result = c89str_find(stbImplSectionClean, "#ifndef STB_SPRINTF_MSVC_MODE", &blockEnd);    /* <-- Will not be included in the replaced code. */
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h implementation section.");
        return -1;
    }

    sectionToMoveFromHeadToImpl = c89str_cat(sectionToMoveFromHeadToImpl, NULL, "\n");
    stbImplSectionClean = c89str_replace(stbImplSectionClean, NULL, blockBeg, blockEnd - blockBeg, sectionToMoveFromHeadToImpl, (size_t)-1);


    /*
    There's a part at the bottom of the implementation that undefines some sized types that
    we just deleted. Need to get rid of this section.
    */
    result = c89str_find(stbImplSectionClean, "#undef stbsp__uint16", &blockBeg);
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h implementation section.");
        return -1;
    }
    result = c89str_find(stbImplSectionClean, "#undef STBSP__UNALIGNED", &blockEnd);    /* <-- Will not be included in the replaced code. */
    if (result != C89STR_SUCCESS) {
        printf("Could not find required section in stb_sprintf.h implementation section.");
        return -1;
    }

    stbImplSectionClean = c89str_replace(stbImplSectionClean, NULL, blockBeg, blockEnd - blockBeg, "", 0);


    /* Now we can do some mass renaming of namespaces. */
    replace_stbsp_namespaces(&stbHeadSectionClean);
    replace_stbsp_namespaces(&stbImplSectionClean);

    /* Comments need to be stripped out because they use // style comments which we're not supporting in c89str. */
    remove_comments(&stbHeadSectionClean);
    remove_comments(&stbImplSectionClean);

    /* Do some basic style cleanup. This is just converting things from "char *" to "char*", etc. just for consistency with the main library. */
    style_cleanup(&stbHeadSectionClean);
    style_cleanup(&stbImplSectionClean);

    /* Now just trim our sections just to clean them up. */
    stbHeadSectionClean = c89str_trim(stbHeadSectionClean, NULL);
    stbImplSectionClean = c89str_trim(stbImplSectionClean, NULL);



    /* We now need to replace the relevant sections in c89str.h. */
    c89str c89strNewFileContent = c89str_new(NULL, c89strFileContent);
    c89str_replace_range_tagged(&c89strNewFileContent, NULL, "/* beg stb_sprintf.h */", "/* end stb_sprintf.h */", stbHeadSectionClean, NULL, NULL, C89STR_TRUE);
    c89str_replace_range_tagged(&c89strNewFileContent, NULL, "/* beg stb_sprintf.c */", "/* end stb_sprintf.c */", stbImplSectionClean, NULL, NULL, C89STR_TRUE);



    /* Now we can output the file. */
    c89str_open_and_write_text_file(argv[1], c89strNewFileContent);

    return 0;
}
