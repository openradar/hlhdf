/**
 * Common functions for working with an HDF5 file through the HL-HDF API.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-10
 */
#ifndef HLHDF_H
#define HLHDF_H
#include <hdf5.h>
#include "hlhdf_types.h"
#include "hlhdf_node.h"
#include "hlhdf_nodelist.h"
#include "hlhdf_read.h"
#include "hlhdf_write.h"
#include "hlhdf_compound.h"

/**
 * Define for FALSE unless it already has been defined.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 * Define for TRUE unless it already has been defined.
 */
#ifndef TRUE
#define TRUE 1
#endif

/**
 * @defgroup hlhdf_c_apis HLHDF C-API Reference Manual
 * This is the currently provided APIs available in C. This group
 * is a subset of all defined functions and if more information on
 * all source code is wanted, please refer to the files section of
 * the documentation.
 */

/**
 * Disables error reporting.
 * @ingroup hlhdf_c_apis
 */
void disableErrorReporting(void);

/**
 * Enables error reporting
 * @ingroup hlhdf_c_apis
 */
void enableErrorReporting(void);

/**
 * Initializes the HLHDF handler functions.
 * <b>This always needs to be done before doing anything else when using HLHDF.</b>
 * @ingroup hlhdf_c_apis
 */
void initHlHdf(void);

/**
 * Toggles the debug mode for HLHDF. Possible values of flag are:
 * <ul>
 *   <li>0 = No debugging</li>
 *   <li>1 = Debug only the HLHDF library</li>
 *   <li>2 = Debug both HLHDF and HDF5 library</li>
 * </ul>
 * @ingroup hlhdf_c_apis
 * @param[in] flag the level of debugging
 */
void debugHlHdf(int flag);

/**
 * Verifies if the provided filename is a valid HDF5 file or not.
 * @ingroup hlhdf_c_apis
 * @param[in] filename the full path of the file to check
 * @return TRUE if file is an HDF5 file, otherwise FALSE
 */
int isHdf5File(const char* filename);

/**
 * Creates a file property instance that can be passed on to createHlHdfFile when
 * creating a HDF5 file.
 * @ingroup hlhdf_c_apis
 * @return the allocated file property instance, NULL on failure. See @ref freeHL_fileCreationProperty for deallocation.
 */
HL_FileCreationProperty* createHlHdfFileCreationProperty(void);

/**
 * Deallocates the HL_FileCreationProperty instance.
 * @ingroup hlhdf_c_apis
 * @param[in] prop The property to be deallocated
 */
void freeHL_fileCreationProperty(HL_FileCreationProperty* prop);

/**
 * Calculates the size in bytes of the provided @ref ValidFormatSpecifiers "format specifiers".
 * The exception is string and compound type since they needs to be analyzed to get the size.
 * @ingroup hlhdf_c_apis
 * @param[in] format The @ref ValidFormatSpecifiers "format specifier".
 * @return the size in bytes or -1 on failure.
 */
int whatSizeIsHdfFormat(const char* format);

/**
 * Verifies if the format name is supported by HLHDF.
 * Does not see string and compound as a supported format since they are not
 * possible to determine without a hid.
 * @ingroup hlhdf_c_apis
 * @param[in] format the format name string
 * @return TRUE if it is supported, otherwise FALSE.
 */
int isFormatSupported(const char* format);

/**
 * Returns the format specifier for the provided format string.
 * @param[in] format the format name
 * @return the format specifier. HLHDF_UNDEFINED is also returned on error
 */
HL_FormatSpecifier HL_getFormatSpecifier(const char* format);

/**
 * Returns the string representation of the provided specifier.
 * @param[in] specifier - the specifier
 * @return NULL if not a valid specifier, otherwise the format.
 */
const char* HL_getFormatSpecifierString(HL_FormatSpecifier specifier);

/**
 * Creates an allocated and initialized instance of HL_Compression.
 * @ingroup hlhdf_c_apis
 * @param[in] type the compression type to use
 * @return the created instance, NULL on failure.
 */
HL_Compression* newHL_Compression(HL_CompressionType type);

/**
 * Creates a copy of the provided HL_Compression instance.
 * @ingroup hlhdf_c_apis
 * @param[in] inv the instance to be cloned
 * @return the cloned instance or NULL if parameter was NULL or memory not could be allocated.
 */
HL_Compression* dupHL_Compression(HL_Compression* inv);

/**
 * Initializes a HL_Compressiosn instance.
 * @ingroup hlhdf_c_apis
 * @param[in] inv The compression object to be initialized
 * @param[in] type the type of compression the object should be initialized with
 */
void initHL_Compression(HL_Compression* inv,HL_CompressionType type);

/**
 * Deallocates the HL_Compression instance.
 * @ingroup hlhdf_c_apis
 * @param inv The instance that should be deallocated
 */
void freeHL_Compression(HL_Compression* inv);

#endif
