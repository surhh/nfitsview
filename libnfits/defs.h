#ifndef LIBNFITS_DEFS_H
#define LIBNFITS_DEFS_H

#include <cstdint>
#include <iostream>

// this typedef may be used for defining callback functions used in the threads in the future coding
typedef     int32_t (*CallbackFunctionPtr)       (int32_t, void*);

static int32_t percentCallbackFunctionSample(int32_t a_percent, void* a_buffer)
{
    std::string progress = "Progress... : " + std::to_string(a_percent);

    return a_percent;
}

//#define DEBUG_MODE                            //// enabling/disabling the debug logging used in libnfits::LOG()

#define ENABLE_OPENMP                           //// enabling/disabling the usage of OpenMP
                                                //// Currently OpenMP is not suitable for some of the algorithms

#define OPENMP_THREADS_DISABLE_NUMBER           (0) //// Number of excluded OpenMP threads - default here is 2

#define ENABLE_FILE_MAPPING_FILE_LOADING        //// enabling for mapping file into RAM or legacy file reading

#define LIBNFITS_MAJOR_VERSION                  3
#define LIBNFITS_MINOR_VERSION                  5

#define	FITS_BSCALE_DEFAULT_VALUE               (1.0)
#define	FITS_BZERO_DEFAULT_VALUE                (0.0)

#define FITS_BLOCK_SIZE                         (2880)
#define FITS_HEADER_RECORD_SIZE                 (80)
#define FITS_KEYWORD_END_POS                    (8)
#define FITS_MIN_KEYWORD_VALUE_STR_LENGTH       (10)
#define FITS_MIN_RECORD_ASCII_CHAR              (0x20)
#define FITS_MAX_RECORD_ASCII_CHAR              (0x7e)
#define FITS_MIN_KEYWORD_ASCII_CHAR1            (0x30)
#define FITS_MAX_KEYWORD_ASCII_CHAR1            (0x39)
#define FITS_MIN_KEYWORD_ASCII_CHAR2            (0x65)
#define FITS_MAX_KEYWORD_ASCII_CHAR2            (0x90)
#define FITS_KEYWORD_ASCII_CHAR3                (0x2d)
#define FITS_KEYWORD_ASCII_CHAR4                (0x5f)

#define FITS_UNDEFINED_VALUE                    (-1)

#define FITS_COMPRESS_MEMORY_CHUNK_MB           (64)                // 64 MB
#define FITS_COMPRESS_MEMORY_CHUNK_SIZE         (FITS_COMPRESS_MEMORY_CHUNK_MB * 1024 * 1024)

#define FITS_HEADER_RECORD_ASSIGNMENT_CHAR      '='
#define FITS_PADDING_SPACE_CHAR                 ' '
#define FITS_QUOTE_CHAR                         '\''
#define FITS_DOUBLE_QUOTE_CHAR                  "\'\'"
#define FITS_COMMENT_START_CHAR                 '/'
#define FITS_UNDEFINED_STR_VALUE                ""
#define FITS_VALUE_CONTINUE_CHAR                '&'

#define FITS_GENERAL_SUCCESS                    (1)
#define FITS_ONLY_KEYWORD_SUCCESS               (2)
#define FITS_GENERAL_ERROR                      (0)
#define FITS_PNG_EXPORT_ERROR                   (-1)
#define FITS_PNG_HDU_NOT_IMAGE_ERROR            (-2)
#define FITS_EMPTY_STRING_ERROR                 (-1)
#define FITS_RECORD_NOT_FOUND                   (-1)
#define FITS_RECORD_SYNTAX_ERROR                (-2)
#define FITS_RECORD_SIZE_ERROR                  (-4)

#define FITS_MSG_GENERAL_SUCCESS                "Operation finished successfully"
#define FITS_MSG_ONLY_KEYWORD_SUCCESS           "There is only keyword in the record"
#define FITS_MSG_GENERAL_ERROR                  "General error occurred"
#define FITS_MSG_EMPTY_STRING_ERROR             "The record string is empty"
#define FITS_MSG_RECORD_SYNTAX_ERROR            "Record syntax error"
#define FITS_MSG_GENERAL_RECORD_SIZE_ERROR      "Record size error"
#define FITS_MSG_ERROR_OPENING_FILE             "Error occurred during opening the file or the file is not FITS format!"
#define FITS_MSG_ERROR_TYPE                     "File Open Error"

#define FITS_MEMORY_MAP_FILE_SUCCESS            (1)
#define FITS_MEMORY_MAP_FILE_ERROR              (0)

#define FITS_FILE_OPEN_STAT_SUCCESS             (0)
#define FITS_MEMORY_UNMAP_SUCCESS               (0)

#define FITS_MEMORY_MAP_FILE_IO_ERROR           (-1)
#define FITS_MEMORY_MAP_FILE_OPEN_ERROR         (-2)
#define FITS_MEMORY_MAP_FILE_FSTAT_ERROR        (-4)
#define FITS_MEMORY_MAP_FILE_MAP_ERROR          (-8)
#define FITS_FILE_WRONG_SIZE                    (-16)

#define FITS_GZIP_ERROR                         (0)
#define FITS_COMPRESSIION_GZIP                  "gzip"
#define FITS_COMPRESSIION_ZLIB                  "zlib"

#define FITS_PNG_FILE_CREATE_ERROR              (-1)
#define FITS_PNG_WRITE_STRUCT_CREATE_ERROR      (-2)
#define FITS_PNG_INFO_STRUCT_CREATE_ERROR       (-4)
#define FITS_PNG_PIXEL_DATA_ERROR               (-8)
#define FITS_PNG_WRITE_END_ERROR                (-16)
#define FITS_PNG_TITLE_KEY                      "Title"
#define FITS_PNG_TITLE                          "Generated by nFITSView (libnfits) !"
#define FITS_PNG_DEFAUL_PIXEL_BYTES_NUMBER      (3)
#define FITS_PNG_DEFAULT_PIXEL_DEPTH            (8)

#define FITS_PNG_COLOR_GRAYSCALE                (0)
#define FITS_PNG_COLOR_GRAYSCALE_ALPHA          (4)
#define FITS_PNG_COLOR_PALETTE                  (3)
#define FITS_PNG_COLOR_RGB                      (2)
#define FITS_PNG_COLOR_RGB_ALPHA                (6)
#define FITS_PNG_DEFAULT_COLOR_TYPE             FITS_PNG_COLOR_RGB

#define FITS_HDU_START_ERROR                    (-1)
#define FITS_HDU_OFFSET_ERROR                   (-4)
#define FITS_HDU_PRIMARY_HDU_INDEX              (0)

#define FITS_HDU_TYPE_PRIMARY                        (1)
#define FITS_HDU_TYPE_IMAGE_XTENSION                 (2)
#define FITS_HDU_TYPE_ASCII_TABLE_XTENSION           (4)
#define FITS_HDU_TYPE_BINARY_TABLE_XTENSION          (8)
#define FITS_HDU_TYPE_COMPRESSED_IMAGE_XTENSION      (16)
#define FITS_HDU_TYPE_COMPRESSED_TABLE_XTENSION      (32)
#define FITS_HDU_TYPE_RANDOM_GROUP_RECORDS           (64)

#define FITS_FLOAT_DOUBLE_NO_TRANSFORM                          (0)
#define FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_POSITIVE             (1)
#define FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_NEGATIVE_POSITIVE    (2)
#define FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_NEGATIVE             (3)
#define FITS_NUMBER_OF_TRANSFORMS                               (4)

#define FITS_FLOAT_DOUBLE_RANGE_MIN_ZERO             (0.0)
#define FITS_FLOAT_DOUBLE_RANGE_MAX_ZERO             (0.0)
#define FITS_FLOAT_DOUBLE_RANGE_MIN_NEGATIVE         (-1.0)
#define FITS_FLOAT_DOUBLE_RANGE_MAX_POSITIVE         (1.0)

#define FITS_VALUE_DISTRIBUTION_SEGMENTS_NUMBER      (100)
#define FITS_VALUE_DISTRIBUTION_RANGE_MIN_THREASHOLD (0.01)

#endif // LIBNFITS_DEFS_H
