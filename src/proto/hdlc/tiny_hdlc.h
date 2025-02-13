#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup HDLC_API Tiny HDLC protocol API functions
 * @{
 */

/// \cond
#ifdef CONFIG_ENABLE_FCS32
    typedef uint32_t crc_t;
#else
    typedef uint16_t crc_t;
#endif
/// \endcond


typedef enum
{
    HDLC_CRC_DEFAULT = 0,
    HDLC_CRC_8 = 8,
    HDLC_CRC_16 = 16,
    HDLC_CRC_32 = 32,
    HDLC_CRC_OFF = 0xFF,
} hdlc_crc_t;

/**
 * Structure describes configuration of lowest HDLC level
 * Initialize this structure by 0 before passing to hdlc_init()
 * function.
 */
typedef struct _hdlc_handle_t
{
    /**
     * Send bytes callback user-defined function. This callback
     * must implement physical sending of bytes hw channel.
     * @param user_data user-defined data
     * @param data buffer with data to send over hw channel
     * @param len size of data in bytes to send.
     * @return user callback must return negative value in case of error or
     *         0 if hw device is busy, or positive number - number of bytes
     *         sent.
     */
    int (*send_tx)(void *user_data, const void *data, int len);

    /**
     * User-defined callback, which is called when new packet arrives from hw
     * channel.
     * @param user_data user-defined data
     * @data data pointer to received data
     * @param len size of received data in bytes
     * @return user callback must return negative value in case of error
     *         or 0 value if packet is successfully processed.
     */

    int (*on_frame_read)(void *user_data, void *data, int len);

    int (*on_frame_sent)(void *user_data, const void *data, int len);

    /**
     * Buffer to be used by hdlc level to receive data to
     */
    void *rx_buf;

    /**
     * size of rx buffer
     */
    int rx_buf_size;

    /**
     * crc field type to use on hdlc level.
     * If HDLC_CRC_DEFAULT is passed, crc type will be selected automatically (depending on library configuration),
     * but HDLC_CRC_16 has higher priority.
     */
    hdlc_crc_t crc_type;

    /** User data, which will be passed to user-defined callback as first argument */
    void *user_data;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    /** Parameters in DOXYGEN_SHOULD_SKIP_THIS section should not be modified by a user */
    struct
    {
        uint8_t *data;
        int len;
        int (*state)( struct _hdlc_handle_t *handle, uint8_t *data, int len );
        uint8_t escape;
    } rx;
    struct
    {
        const uint8_t *origin_data;
        const uint8_t *data;
        int len;
        crc_t crc;
        uint8_t escape;
        int (*state)( struct _hdlc_handle_t *handle );
    } tx;
#endif
} hdlc_struct_t, *hdlc_handle_t;

/**
 * Initializes hdlc level and returns hdlc handle or NULL in case of error.
 *
 * @param hdlc_info pointer to hdlc_struct_t structure, which defines user-specific configuration
 * @return handle to hdlc instance or NULL.
 * @warning hdlc_info structure passed to the function must be allocated all the time.
 */
hdlc_handle_t hdlc_init( hdlc_struct_t *hdlc_info );

/**
 * Shutdowns all hdlc activity
 *
 * @param handle handle to hdlc instance
 */
int hdlc_close( hdlc_handle_t handle );

/**
 * Resets hdlc state. Use this function, if hw error happened on tx or rx
 * line, and this requires hardware change, and cancelling current operation.
 *
 * @param handle handle to hdlc instance
 */
void hdlc_reset( hdlc_handle_t handle );

/**
 * Processes incoming data. Implementation of reading data from hw is user
 * responsibility. If hdlc_run_rx() returns value less than size of data
 * passed to the function, then hdlc_run_rx() must be called later second
 * time with the pointer to and size of not processed bytes.
 *
 * @param handle handle to hdlc instance
 * @param data pointer to incoming data to process
 * @param len size of received data in bytes
 * @return number of bytes actually processed.
 */
int hdlc_run_rx( hdlc_handle_t handle, void *data, int len );

/**
 * Runs transmission at hdlc level. If there is frame to send, or
 * send is in progress, this function will call send_tx() callback
 * multiple times. If send_tx() callback reports 0, that means that
 * hw device is busy, then hdlc_run_tx() will return immediately and
 * hdlc_run_tx() must be called later next time.
 *
 * @param handle handle to hdlc instance
 * @return negative value in case of error
 *         0 if hw is busy, or no data is waiting for sending
 *         positive number of bytes passed to hw channel.
 */
int hdlc_run_tx( hdlc_handle_t handle );

/**
 * Puts next frame for sending.
 *
 * @param handle handle to hdlc instance
 * @param data pointer to new data to send
 * @param len size of data to send in bytes
 * @return 0 if output queue is busy
 *         1 if pointer to data is passed to output queue
 * @warning buffer with data must be available all the time until
 *          data are actually sent to tx hw channel
 */
int hdlc_send( hdlc_handle_t handle, const void *data, int len );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

