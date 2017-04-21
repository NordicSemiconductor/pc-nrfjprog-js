#ifndef EnRFDLL_FUNC_H
#define EnRFDLL_FUNC_H


#include <stdint.h>
#include "DllCommonDefinitions.h"

/*  HiLvlnRFJProg interface overview:
 *
 *  nrfjprogdll_err_t HiLvlnRFJ_dll_get_version     (uint32_t * major, uint32_t * minor, uint32_t * revision);
 *  nrfjprogdll_err_t HiLvlnRFJ_dll_open            (log_callback * cb);
 *  void              HiLvlnRFJ_dll_close           (void);
 *  nrfjprogdll_err_t HiLvlnRFJ_is_dll_open         (bool * is_opened);
 *  nrfjprogdll_err_t HiLvlnRFJ_get_connected_probes(uint32_t serial_numbers[], uint32_t serial_numbers_len, uint32_t * num_available, const char * jlink_path);
 *  
 *  nrfjprogdll_err_t HiLvlnRFJ_probe_init          (Probe_handle_t * debug_probe, uint32_t snr, const char * qspi_ini_path, const char * jlink_path);
 *  nrfjprogdll_err_t HiLvlnRFJ_probe_uninit        (Probe_handle_t * debug_probe);
 *  nrfjprogdll_err_t HiLvlnRFJ_probe_get_snr       (Probe_handle_t debug_probe, uint32_t * snr);
 *  
 *  nrfjprogdll_err_t HiLvlnRFJ_get_device_family   (Probe_handle_t debug_probe, device_family_t * family);
 *  nrfjprogdll_err_t HiLvlnRFJ_get_device_version  (Probe_handle_t debug_probe, device_version_t * device);
 *  nrfjprogdll_err_t HiLvlnRFJ_program             (Probe_handle_t debug_probe, const char * hex_path, program_options_t * program_options, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_read_to_file        (Probe_handle_t debug_probe, const char * hex_path, read_options_t * read_options, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_verify              (Probe_handle_t debug_probe, const char * hex_path, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_erase               (Probe_handle_t debug_probe, erase_mode_t erase_mode, uint32_t start_adress, uint32_t end_adress, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_recover             (Probe_handle_t debug_probe, progress_callback * fp);
 *  
 *  nrfjprogdll_err_t HiLvlnRFJ_read                (Probe_handle_t debug_probe, uint32_t addr, uint8_t * data, uint32_t data_len, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_read_u32            (Probe_handle_t debug_probe, uint32_t addr, uint32_t * data, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_write               (Probe_handle_t debug_probe, uint32_t addr, const uint8_t * data, uint32_t data_len, progress_callback * fp);
 *  nrfjprogdll_err_t HiLvlnRFJ_write_u32           (Probe_handle_t debug_probe, uint32_t addr, const uint32_t data, progress_callback * fp);
 *  
 *  nrfjprogdll_err_t HiLvlnRFJ_reset               (Probe_handle_t debug_probe, reset_action_t reset_action);
 *  nrfjprogdll_err_t HiLvlnRFJ_go                  (Probe_handle_t debug_probe);
 */

#if defined(__cplusplus)
extern "C" {
#endif

/* Expected log function prototype for logging operations. */
typedef void progress_callback(uint32_t step, uint32_t total_steps, const char * process);
typedef void log_callback(const char * msg);

typedef void * Probe_handle_t;  // forward declare to "something_t", hide void *

typedef enum {
    ERASE_NONE = 0,                 /* Do nothing. */
    ERASE_ALL = 1,                  /* Erase whole chip. */
    ERASE_SECTOR = 2,               /* Erase specified sectors, excluding UICR. */
    ERASE_SECTOR_AND_UICR = 3       /* Erase specified sectors, with UICR support. */
}erase_mode_t;

typedef struct program_options_s {
    bool verify;                    /* Should the memory be verified? */
    erase_mode_t chip_erase_mode;   /* Select pre-program erase mode for internal flash memories. */
    erase_mode_t qspi_erase_mode;   /* Select pre-program erase mode for external QSPI memories. */
    bool reset;                     /* Select whether program should do post-program Reset. */
} program_options_t;

typedef struct read_options_s {
    bool readram;
    bool readcode;
    bool readuicr;
    bool readqspi;
} read_options_t;


/**
 * @brief  Get the HilvlNrfjprog DLL  version information.
 * 
 * @param   major                               Pointer for storing dll major version.
 * @param   minor                               Pointer for storing dll minor version.
 * @param   revision                            Pointer for storing dll revision.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The major parameter is NULL.
 *                                              The minor parameter is NULL.
 *                                              The revision parameter is NULL.
 */
nrfjprogdll_err_t HiLvlnRFJ_dll_get_version     (uint32_t * major, uint32_t * minor, uint32_t * revision);


/**
 * @brief   Opens the low-level device-agnostic NRFJPROG DLL and sets the log callback.
 *
 * @pre     Before the execution of this function, the dll must not be open. To close the dll, see HiLvlnRFJ_dll_close() function.
 *
 * @post    After the execution of this function, the Nrfjprog DLL pointers will be loaded and some memory reserved. To unload the pointers and free the memory, see NRFJPROG_close_dll() function.
 *
 * @param   cb                                        Callback for reporting informational and error messages.
 *
 * @retval  SUCCESS
 * @retval  INVALID_OPERATION                         The NRFJPROG_open_dll() function has already been called.
 * @retval  NRFJPROG_SUB_DLL_NOT_FOUND                The Dll could not find the NRFJPROG DLL, please reinstall nRF tools.
 * @retval  NRFJPROG_SUB_DLL_COULD_NOT_BE_OPENED      An error occurred while opening the NRFJPROG DLL.
 * @retval  NRFJPROG_SUB_DLL_COULD_NOT_LOAD_FUNCTIONS A required function could not be loaded from the DLL.
 */
nrfjprogdll_err_t HiLvlnRFJ_dll_open            (log_callback * cb);


/**
 * @brief   Closes and frees the NRFJPROG DLL.
 *
 * @details Closes and frees the NRFJPROG DLL. This function needs to be called before exiting if HiLvlnRFJ_dll_open() has been called.
 *          After the execution of this function, the device CPU will not change its state from running or halted.
 *
 * @post    After the execution of this function, the NRFJPROG DLL function pointers will be unloaded and the reserved memory freed. To open the dll, see HiLvlnRFJ_dll_open() function.
 */
void              HiLvlnRFJ_dll_close           (void);


/**
 * @brief   Checks if the NRFJPROG DLL is open.
 *
 * @details Checks if the HiLvlnRFJ_dll_open() function has been called since the last call to HiLvlnRFJ_dll_close() or since the loading of this dll.
 *
 * @param   is_opened                           Pointer of the location to store the result.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The is_opened parameter is NULL.
 */
nrfjprogdll_err_t HiLvlnRFJ_is_dll_open         (bool * is_opened);


/**
 * @brief   Returns a list of serial numbers of connected USB J-Link emulators.
 *
 * @details This function asks the NRFJPROG DLL how many USB J-Link emulators are connected to
 *          the PC, and writes that value into the num_available parameter. It also copies
 *          up to serial_numbers_len serial numbers into the serial_numbers array parameter.
 *          Since this function can be called before a probe is initialized, the path to the JLinkARM DLL can be supplied.
 *          The jlink_path parameter can be passed as NULL to automatically use the newest version of the JLinkARM DLL.
 *
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.
 *
 * @param   serial_numbers                      Array in which to store the enumerated serial numbers.
 * @param   serial_numbers_len                  Number of uint32_t values that can be stored in the
 *                                              serial_numbers array (may be zero).
 * @param   num_available                       The number of serial numbers that were enumerated.
 * @param   jlink_path                          Path to the JLinkARM DLL.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The serial_numbers parameter is NULL.
 *                                              The num_available parameter is NULL.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  OUT_OF_MEMORY                       Could not allocate buffer for reading serial numbers
 */
nrfjprogdll_err_t HiLvlnRFJ_get_connected_probes(uint32_t serial_numbers[], uint32_t serial_numbers_len, uint32_t * num_available, const char * jlink_path);


/**
 * @brief   Allocates and initializes a new probe connection handle.
 *
 * @details This function allocates a new probe_handle and attempts to initialize it. 
 *          If the target device is locked, initialization will fail, but the object is not discarded.
 *          A recovery operation will then be required in order to initialize the object before any other operation can be performed.
 *          See HiLvlnRFJ_recover() for how to recover a locked device.
 *
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 *
 * @during  During the execution of this function, if the device was not in debug interface mode an unavoidable pin reset will occur. The pin reset will not occur if the debugger used is the Nordic's JLink On-Board-Debugger.
 *
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset(), HiLvlnRFJ_go() functions.
 *
 * @param   debug_probe                         Pointer to probe handle to initialize.
 * @param   serial_number                       Serial number of the emulator to connect to.
 * @param   jlink_path                          Path to the JLinkARM DLL.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   debug_probe pointer is NULL.
 *                                              qspi_init_params is NULL.
 * @retval  INVALID_OPERATION                   DLL is closed.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  OUT_OF_MEMORY                       Could not allocate buffer for reading serial numbers
 * @retval  EMULATOR_NOT_CONNECTED              The emulator serial_number is not connected to the PC.
 */
nrfjprogdll_err_t HiLvlnRFJ_probe_init          (Probe_handle_t * debug_probe, uint32_t serial_number, const qspi_init_params_t * qspi_mem_config, const char * jlink_path);


/**
 * @brief   Deallocates a probe connection handle.
 *
 * @details This function deallocates the memory allocated for the provided handle debug_probe, and clears it by writing it to NULL.
 *
 * @pre     Before the execution of this function, the provided handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init().
 *
 * @param   debug_probe                         Pointer to probe handle to uninitialize.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   debug_probe pointer is NULL.
 *                                              debug_probe is not initialized.
 */
nrfjprogdll_err_t HiLvlnRFJ_probe_uninit        (Probe_handle_t * debug_probe);


/**
* @brief   Sets up the probe handle for QSPI communication.
*
* @details Reads the configuration stored in the supplied qspi_ini_path and stores it in the probe object. 
*          This information will later be used to initialize the QSPI peripheral on demand.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
*
* @post    After the execution of this function, the probe will be able to perform QSPI operations.
*
* @param   debug_probe                         Probe handle.
* @param   qspi_ini_path                       Path to the .ini containing initialization parameters for the QSPI memory interface.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The debug_probe parameter is null.
*                                              The qspi_ini_path parameter is null.
*                                              Could not find the supplied .ini file.
*                                              The supplied .ini file has syntax errors.
*                                              One of the QSPI parameters failed verification.
* @retval  INVALID_DEVICE_FOR_OPERATION        The connected device does not support QSPI.
 */
nrfjprogdll_err_t HiLvlnRFJ_probe_setup_qspi    (Probe_handle_t debug_probe, const char * qspi_ini_path);


/**
* @brief   Reads the serial number from the probe handle.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 

* @param   debug_probe                         Probe handle.
* @param   serial_number                       Pointer to where serial number value should be stored.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The debug_probe parameter is null.
* @retval                                      The serial_number parameter is null.
 */
nrfjprogdll_err_t HiLvlnRFJ_probe_get_snr       (Probe_handle_t debug_probe, uint32_t * serial_number);


/**
* @brief   Reads the stored device family value from the probe handle.
*
* @details Returns the stored device family value. This function does not query the device.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
*
* @param   debug_probe                         Probe handle.
* @param   family                              Pointer to where device family should be stored.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The debug_probe parameter is null.
* @retval                                      The family parameter is null.
 */
nrfjprogdll_err_t HiLvlnRFJ_get_device_family   (Probe_handle_t debug_probe, device_family_t * family);


/**
* @brief   Reads the stored device type value from the probe handle.
*
* @details Returns the stored device type value. This function does not query the device.
*          If the probe handle failed initialization because of protection, the returned device family value is unknown.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
*
* @param   debug_probe                         Probe handle.
* @param   family                              Pointer to where device family should be stored.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The debug_probe parameter is null.
* @retval                                      The device parameter is null.
 */
nrfjprogdll_err_t HiLvlnRFJ_get_device_version  (Probe_handle_t debug_probe, device_version_t * device);


/**
 * @brief   Programs the provided .hex file  onto the device connected to the debug_probe.
 *
 * @details Programs a hex file to the device, can optionally also verify the programming, and perform erase operations on whole device or specific pages.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     If a QSPI operation is to be performed, QSPI must be enabled in the probe. To enable QSPI, see HiLvlnRFJ_probe_setup_qspi().
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.
 * @pre     Before the execution of this function, access port protection must be disabled. To disable access port protection, see HiLvlnRFJ_recover() function.
 * 
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset(), HiLvlnRFJ_go() functions.
 *
 * @param   debug_probe                         Probe handle.
 * @param   hex_path                            Path to file to be programmed.
 * @param   program_options                     Pointer to struct containing programming parameters.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The hex file path is an empty string.
 *                                              The hex file cannot be found.
 *                                              The hex file cannot be read.
 *                                              The program_options pointer is NULL.
 *                                              The debug_probe pointer is NULL.
 *                                              The hex file contains data in nonexistent regions.
 *                                              The hex file contains data that crosses memory barriers.
 * @retval  INVALID_OPERATION                   The hex file was not valid.
 *                                              The hex file cannot be opened.
 *                                              UICR erase requested in ERASE_SECTOR mode.
 *                                              ERASE_SECTOR_AND_UICR attempted on QSPI memory.
 *                                              QSPI operation on non-QSPI enabled device.
 *                                              QSPI has not been enabled for this probe.
 * @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available because the device is readback protected.
 *                                              The hex file contains data in a write protected region.
 * @retval  INVALID_DEVICE_FOR_OPERATION        The device does not support an attempted operation.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 *                                              Attempted to access unpowered RAM.
 * @retval  OUT_OF_MEMORY                       Could not allocate programming buffers.
 * @retval  EMULATOR_NOT_CONNECTED              The emulator serial_number is not connected to the PC.
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 * @retval  NVMC_ERROR                          Flash operation failed.
 * @retval  VERIFY_ERROR                        Program verification failed.
 */
nrfjprogdll_err_t HiLvlnRFJ_program             (Probe_handle_t debug_probe, const char * hex_path, program_options_t * program_options, progress_callback * fp);


/**
* @brief   Reads the specified memory to the provided .hex file.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
* @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
* @pre     If a QSPI operation is to be performed, QSPI must be enabled in the probe. To enable QSPI, see HiLvlnRFJ_probe_setup_qspi().
* @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.
* @pre     Before the execution of this function, access port protection must be disabled. To disable access port protection, see HiLvlnRFJ_recover() function.
* 
* @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
* @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset(), HiLvlnRFJ_go() functions.
*
* @param   debug_probe                         Probe handle.
* @param   hex_path                            Path to output file.
* @param   read_options                        Pointer to struct containing read parameters.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The hex file path is an empty string.
*                                              The hex file cannot be overwritten.
*                                              The read_options pointer is NULL.
*                                              The debug_probe pointer is NULL.
*                                              The hex file cannot be opened for writing.
* @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
* @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
* @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
*                                              A required function could not be loaded from the DLL.
* @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
*                                              The address to read is in unpowered RAM.
* @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available because the device is readback protected.
* @retval  INVALID_DEVICE_FOR_OPERATION        The connected device does not support an attempted operation.
* @retval  OUT_OF_MEMORY                       Could not write to the hex file.
*                                              Could not allocate hex file buffer.
*                                              Could not extend hex file buffer.
*                                              Could not save the hex file.
*/
nrfjprogdll_err_t HiLvlnRFJ_read_to_file        (Probe_handle_t debug_probe, const char * hex_path, read_options_t * read_options, progress_callback * fp);


/**
* @brief   Reads the specified memory to the provided .hex file.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
* @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
* @pre     If a QSPI operation is to be performed, QSPI must be enabled in the probe. To enable QSPI, see HiLvlnRFJ_probe_setup_qspi().
* @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.
* @pre     Before the execution of this function, access port protection must be disabled. To disable access port protection, see HiLvlnRFJ_recover() function.
* 
* @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
* @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset(), HiLvlnRFJ_go() functions.
*
* @param   debug_probe                         Probe handle.
* @param   hex_path                            Path to file to verify against.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The hex file path is an empty string.
*                                              The hex file cannot be read.
*                                              The debug_probe pointer is NULL.
* @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
* @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
* @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
*                                              A required function could not be loaded from the DLL.
* @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
*                                              The address to read is in unpowered RAM.
* @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available because the device is readback protected.
* @retval  INVALID_DEVICE_FOR_OPERATION        The connected device does not support an attempted operation.
* @retval  OUT_OF_MEMORY                       Could not allocate program buffers.
* @retval  VERIFY_ERROR                        Program verification failed.
*/
nrfjprogdll_err_t HiLvlnRFJ_verify              (Probe_handle_t debug_probe, const char * hex_path, progress_callback * fp);


/**
* @brief   Erases the pages containing the specified flash memory region.
*
* @details Performs an erase operation on the passed memory region. Code, UICR and QSPI memory areas are supported.
*          Any one of ERASE_ALL, ERASE_SECTOR and ERASE_SECTOR_UICR can be performed. If ERASE_ALL is requested, start_address and end_address parameters are not used.
*          In ERASE_SECTOR and ERASE_SECTOR_UICR mode, the whole sectors containing start_address and end_address are erased, in addition to any intermediates.
*          UICR can only be erased in ERASE_SECTOR_UICR mode. In ERASE_SECTOR_UICR both Code and UICR sectors can be erased.
*          If the address to erase is in the xip memory area, only ERASE_ALL and ERASE_SECTOR modes are supported.
*          ERASE_NONE does nothing.
*
* @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
* @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
* @pre     If a QSPI operation is to be performed, QSPI must be enabled in the probe. To enable QSPI, see HiLvlnRFJ_probe_setup_qspi().
* @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.
* @pre     Before the execution of this function, access port protection must be disabled. To disable access port protection, see HiLvlnRFJ_recover() function.
*
* @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
* @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset(), HiLvlnRFJ_go() functions.
*
* @param   debug_probe                         Probe handle.
* @param   erase_mode                          Erase action selector.
* @param   start_address                       Start address for erase action.
* @param   end_address                         End address for erase action.
*
* @retval  SUCCESS
* @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
* @retval  INVALID_OPERATION                   UICR erase requested in ERASE_SECTOR mode.
*                                              ERASE_SECTOR_AND_UICR attempted on QSPI memory.
*                                              QSPI operation on non-QSPI enabled device.
*                                              QSPI has not been enabled for this probe.
* @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
* @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
* @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
*                                              A required function could not be loaded from the DLL.
* @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available because the device is readback protected.
* @retval  NVMC_ERROR                          Flash operation failed.
* @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
*/
nrfjprogdll_err_t HiLvlnRFJ_erase               (Probe_handle_t debug_probe, erase_mode_t erase_mode, uint32_t start_address, uint32_t end_address, progress_callback * fp);


/**
 * @brief   Recovers the device.
 *
 * @details This operation attempts to recover the device and leave it as it was when it left Nordic factory. It will attempt to
 *          connect, erase all user available flash, halt and eliminate any protection. Note that this operation may take up to 30 s
 *          if the device was readback protected. Note as well that this function only affects flash and CPU, but does not reset or 
 *          stop any peripheral or oscillator source. The operation will therefore leave the watchdog still operational.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset() and HiLvlnRFJ_go() functions.
 * @post    After the execution of this function, all device RAM will be powered.
 * @post    After the execution of this function, the device code and UICR flash will be erased.
 * @post    After the execution of this function, if the device was readback protected, the device RAM will be erased.
 * @post    After the execution of this function, if the device was readback protected, the device will no longer be readback protected.
 * @post    After the execution of this function, the POWER.RESETREAS register will be cleared.
 *
 * @param   debug_probe                         Probe handle.

 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  NVMC_ERROR                          Flash operation failed.
 * @retval  NOT_AVAILABLE_BECAUSE_MPU_CONFIG    The operation is not available due to the MPU configuration. The operation is not available due to the presence of Pre-Programmed Factory Code (PPFC). 
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_recover             (Probe_handle_t debug_probe, progress_callback * fp);


/**
 * @brief   Reads data_len bytes from the device starting at the given address.
 *
 * @details Reads data_len bytes from the device starting at the given addr without verifying that the addresses are accessible or even
 *          exist. Note that if the target address is in unpowered RAM, the operation will fail. 
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset() and HiLvlnRFJ_go() functions.
 *
 * @param   debug_probe                         Probe handle.
 * @param   address                             Start address of the region read.
 * @param   data                                Pointer to an array to read to.
 * @param   data_len                            Number of bytes to read.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 *                                              The Data pointer is NULL.
 *                                              Attempted to read out of QSPI area.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  NVMC_ERROR                          Flash operation failed.
 * @retval  RAM_IS_OFF_ERROR                    Attempted to read powered-down RAM.
 * @retval  NOT_AVAILABLE_BECAUSE_MPU_CONFIG    The operation is not available due to the MPU configuration. The operation is not available due to the presence of Pre-Programmed Factory Code (PPFC). 
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_read                (Probe_handle_t debug_probe, uint32_t addr, uint8_t * data, uint32_t data_len, progress_callback * fp);


/**
 * @brief   Reads one uint32_t from the given address.
 *
 * @details Reads one uint32_t data from the given address without verifying that the address is accessible or even exists.
 *          Reads need to be 32-bit aligned. Note that if the target address is in unpowered RAM, the operation will fail.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset() and HiLvlnRFJ_go() functions.
 *
 * @param   debug_probe                         Probe handle.
 * @param   address                             Start address of the region to read.
 * @param   data                                Pointer to where the read data should be written.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 *                                              The Data pointer is NULL.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  NVMC_ERROR                          Flash operation failed.
 * @retval  RAM_IS_OFF_ERROR                    Attempted to read powered-down RAM.
 * @retval  NOT_AVAILABLE_BECAUSE_MPU_CONFIG    The operation is not available due to the MPU configuration. The operation is not available due to the presence of Pre-Programmed Factory Code (PPFC). 
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_read_u32            (Probe_handle_t debug_probe, uint32_t address, uint32_t * data, progress_callback * fp);


/**
 * @brief   Writes data from the array into the device starting at the given address.
 *
 * @details Writes data_len bytes from the data array into the device starting at the given address without verifying that the address is accessible
 *          or even exists. If address is in flash or UICR, it will control the NVMC in order to write into flash.
 *          Writes need to be 32-bit aligned. Note that if the target address is in unpowered RAM, the operation will fail.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset() and HiLvlnRFJ_go() functions.
 *
 * @param   debug_probe                         Probe handle.
 * @param   address                             Start address of the region to write to.
 * @param   data                                Pointer to an array with the data to write.
 * @param   data_len                            Length of the data array.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 *                                              The data parameter is NULL.
 *                                              The address is not 32-bit aligned.
 *                                              Attempted to write out of QSPI area.
 * @retval  INVALID_OPERATION                   The target flash is not erased.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  RAM_IS_OFF_ERROR                    Attempted to read disabled RAM.
 * @retval  NVMC_ERROR                          Flash operation failed.
 * @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available due to readback protection.
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_write               (Probe_handle_t debug_probe, uint32_t address, const uint8_t * data, uint32_t data_len, progress_callback * fp);


/**
 * @brief   Writes one uint32_t data into the given address.
 *
 * @details Writes one uint32_t data to the given addr without verifying that the address is accessible or even exists.
 *          If adress is in flash or UICR, it will control the NVMC in order to write into flash. Writes need to be 32-bit aligned.
 *          Note that if the target address is in unpowered RAM, the operation will fail.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @post    After the execution of this function, the device will be in debug interface mode. To exit debug interface mode, see HiLvlnRFJ_reset() function.
 * @post    After the execution of this function, the device CPU will be halted. To unhalt the device CPU, see HiLvlnRFJ_reset() and HiLvlnRFJ_go() functions.
 *
 * @param   debug_probe                         Probe handle.
 * @param   addr                                Address to write to.
 * @param   data                                Value to write.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 *                                              The address is not 32-bit aligned.
 * @retval  INVALID_OPERATION                   The target flash is not erased.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  RAM_IS_OFF_ERROR                    Attempted to write disabled RAM.
 * @retval  NVMC_ERROR                          Flash operation failed.
 * @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available due to readback protection.
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_write_u32           (Probe_handle_t debug_probe, uint32_t addr, const uint32_t data, progress_callback * fp);


/**
 * @brief   Executes a given reset request.
 *
 * @details Executes a system reset, and starts the processor.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @during  During the execution of this function, the emulator mode may be changed to JTAG. If the execution fails, the emulator might be left in JTAG mode. If the execution fails, try again to return to SWD mode.
 *
 * @post    After the execution of this function, the device CPU will be running.
 *
 * @param   debug_probe                         Probe handle.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  JLINKARM_DLL_ERROR                  The JLinkARM DLL function returned an error.
 * @retval  NOT_AVAILABLE_BECAUSE_PROTECTION    The operation is not available due to readback protection.
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_reset               (Probe_handle_t debug_probe);


/**
 * @brief   Starts the device CPU.
 *
 * @details Starts the device CPU with sp and pc as stack pointer and program counter.
 *
 * @pre     Before the execution of this function, the debug_probe handle must be initialized. To initialize the probe, see HiLvlnRFJ_probe_init(). 
 * @pre     Before the execution of this function, the emulator must be physically connected to a powered board.
 * @pre     Before the execution of this function, the dll must be open. To open the dll, see HiLvlnRFJ_dll_open() function.   
 *  
 * @post    After the execution of this function, the device CPU will be running.
 *
 * @param   debug_probe                         Probe handle.
 * @param   sp                                  New stack pointer.
 * @param   pc                                  New program counter.
 *
 * @retval  SUCCESS
 * @retval  INVALID_PARAMETER                   The debug_probe pointer is NULL.
 * @retval  JLINKARM_DLL_TOO_OLD                The version of JLinkARM is lower than the minimum version required.
 * @retval  JLINKARM_DLL_NOT_FOUND              The jlink_path did not yield a usable DLL.
 * @retval  JLINKARM_DLL_COULD_NOT_BE_OPENED    An error occurred while opening the JLinkARM DLL.
 *                                              A required function could not be loaded from the DLL.
 * @retval  CANNOT_CONNECT                      It is impossible to connect to any nRF device.
 */
nrfjprogdll_err_t HiLvlnRFJ_run                  (Probe_handle_t debug_probe, uint32_t pc, uint32_t sp);

#if defined(__cplusplus)
}
#endif

#endif  /* EnRFDLL_FUNC_H */

