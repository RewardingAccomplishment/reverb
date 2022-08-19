/**
 * @file    reverb.c
 * @author  Marcin Sosnowski (marcin.sosnow@gmail.com)
 * @brief   simple reverb effect implementation
 *
 * @version 0.1
 * @date    2022-08-19
 *
 * @copyright Copyright (c) copyright GNU Public License.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reverb.h>

#define ALL_PASS(suffix)              \
    reverb_get(&x, &y, m_##suffix);   \
    log_xy(x, y, __func__, __LINE__); \
    ret = all_pass(ret, x, y, g_##suffix);

#define COMB(suffix)                  \
    reverb_get(&x, &y, m_##suffix);   \
    log_xy(x, y, __func__, __LINE__); \
    ret += comb(sample, x, y, g_##suffix);

typedef struct
{
    int32_t *samples_y;
    int32_t *samples_x;
    uint16_t head;
    uint16_t tail;
} circ_buf_t;

static circ_buf_t buf;
static int samples_max;

/* ----- Log ----------------------------------------------------------------------------------- */
//#define EN_DEBUG
#ifdef EN_DEBUG
static void log_xy(int32_t x, int32_t y, char const *const fname, int line)
{
    printf("%s():%d x %d, y %d\n", fname, line, x, y);
}

static void log_buf(char const *const fname, int line)
{
    printf("%s():%d head %d, tail %d\n", fname, line, buf.head, buf.tail);
    printf("%s():%d ", fname, line);
    for (int i = 0; i < samples_max; i++)
        printf("(%d, %d) ", buf.samples_x[i], buf.samples_y[i]);
    printf("\n");
}

static void log_idx(int32_t idx, char const *const fname, int line)
{
    printf("%s():%d head %d, tail %d, sample_max %d, idx %d\n",
           fname, line, buf.head, buf.tail, samples_max, idx);
}
#else

static void log_xy(int32_t x, int32_t y, char const *const fname, int line)
{
    return;
}
static void log_buf(char const *const fname, int line)
{
    return;
}
static void log_idx(int32_t idx, char const *const fname, int line)
{
    return;
}
#endif

/* ----- Static function ------------------------------------------------------------------------ */
/**
 * @brief Put new element to round buffer of samples
 *
 * @param sample_x input sample
 * @param sample_y output sample
 * @return uint8_t 0 if success
 */
static uint8_t reverb_put(int32_t sample_x, int32_t sample_y)
{
    uint16_t head = buf.head + 1;
    uint16_t tail = buf.tail;
    if (head == samples_max)
        head = 0;

    if (head == tail)
    {
        return 1; // FULL
    }

    buf.samples_x[head] = sample_x;
    buf.samples_y[head] = sample_y;
    buf.head = head;
    log_buf(__func__, __LINE__);
    log_idx(0, __func__, __LINE__);
    return 0;
}

/**
 * @brief Pop returns the x,y sample and move tail.
 *
 * @param x pointer to set tail x-sample
 * @param y pointer to set tail y-sample
 * @return uint8_t 0 if success
 */
static uint8_t reverb_pop(int32_t *x, int32_t *y)
{
    uint16_t head = buf.head;
    uint16_t tail = buf.tail;

    if (head == tail)
    {
        x = 0;
        y = 0;
        return 1; // EMPTY
    }
    log_idx(0, __func__, __LINE__);
    if (((head - tail) != samples_max - 1) && ((tail - head) != 1))
    {
        x = 0;
        y = 0;
        return 2; // NOT READY
    }

    buf.tail += 1;
    if (buf.tail == samples_max)
        buf.tail = 0;

    *x = buf.samples_x[buf.tail];
    *y = buf.samples_y[buf.tail];

    log_buf(__func__, __LINE__);
    log_idx(0, __func__, __LINE__);
    return 0;
}

/**
 * @brief Return the element with idx before of head element.
 *
 * @param x pointer to set x-sample
 * @param y pointer to set y-sample
 * @param idx index of sample before head element to return
 * @return int8_t 0 if success
 */
static uint8_t reverb_get(int32_t *x, int32_t *y, uint16_t idx)
{
    uint16_t head = buf.head;
    uint16_t tail = buf.tail;

    if (idx > samples_max)
    {
        x = 0;
        y = 0;
        return 3; // BADARG
    }

    if (head == tail)
    {
        x = 0;
        y = 0;
        return 1; // EMPTY
    }

    uint16_t buf_idx = (samples_max + buf.head - idx) % samples_max;
    *x = buf.samples_x[buf_idx];
    *y = buf.samples_y[buf_idx];

    log_buf(__func__, __LINE__);
    log_idx(idx, __func__, __LINE__);
    return 0;
}
/**
 * @brief feedback comb filter (see doc)
 *
 * @param sample
 * @param x
 * @param y
 * @param g
 * @return int32_t
 */
static int32_t comb(int16_t sample, int32_t x, int32_t y, float g)
{
    // y[n] = x[n] + g·y[n−M]

    // printf("x = %d y = %d\n", x, y);
    int32_t ret = (int32_t)(y * g);
    // printf("%d, %f = %d\n", y, g, ret);
    return (ret + sample) >> 2;
}

/**
 * @brief allpass filter (see doc)
 *
 * @param sample
 * @param x
 * @param y
 * @param g
 * @return uint32_t
 */
static uint32_t all_pass(int16_t sample, int32_t x, int32_t y, float g)
{
    // y[n] = (−g·x[n]) + x[n−M] + (g·y[n−M])
    int32_t ret = (y - x);
    ret = (int32_t)(ret * g);
    ret += sample;
    return ret >> 2;
}

/* ----- API function ---------------------------------------------------------------------------- */
/**
 * @brief A Schroeder Reverberator called JCRev initialisation
 *
 * @param M - number of samples in buffer
 * @return uint8_t 0 success
 */
uint8_t reverb_init(int M)
{
    if (buf.samples_y || (samples_max > 0))
    {
        return 1;
    }
    samples_max = M + 1;
    buf.samples_x = (int32_t *)malloc(samples_max * sizeof(int32_t));
    memset(buf.samples_x, 0, samples_max * sizeof(int32_t));
    buf.samples_y = (int32_t *)malloc(samples_max * sizeof(int32_t));
    memset(buf.samples_y, 0, samples_max * sizeof(int32_t));
    log_buf(__func__, __LINE__);
    log_idx(0, __func__, __LINE__);
    return 0;
}

void reverb_deinit()
{
    samples_max = 0;
    free(buf.samples_x);
    free(buf.samples_y);
}

#if 1
/**
 * @brief Reverb effect filter: a Schroeder Reverberator called JCRev (see doc)
 *
 * @param sample sample of sound
 * @param m_comb0 should be equal to buffer size (optimisation) so not present
 * @return int16_t
 */
int16_t reverb(int16_t sample, float g_comb0,
               float g_comb1, int16_t m_comb1,
               float g_comb2, int16_t m_comb2,
               float g_comb3, int16_t m_comb3,
               float g_ap0, int16_t m_ap0,
               float g_ap1, int16_t m_ap1,
               float g_ap2, int16_t m_ap2)
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t ret = 0;

    ALL_PASS(ap0);
    ALL_PASS(ap1);
    ALL_PASS(ap2);

    // comb0 is the tail of the buffer so pop have to be use
    reverb_pop(&x, &y);
    log_xy(x, y, __func__, __LINE__);
    ret = comb(sample, x, y, g_comb0);

    COMB(comb1);
    COMB(comb2);
    COMB(comb3);

    reverb_put(sample, ret);
    return ret;
}

#else
// for test comb only
int16_t reverb(int16_t sample, float g_comb0,
               float g_comb1, int16_t m_comb1,
               float g_comb2, int16_t m_comb2,
               float g_comb3, int16_t m_comb3,
               float g_ap0, int16_t m_ap0,
               float g_ap1, int16_t m_ap1,
               float g_ap2, int16_t m_ap2)
{
    int32_t x = 0;
    int32_t y = 0;

    if (reverb_pop(&x, &y))
    {
        // printf("Reverb get empty buffer.\n");
    }
    log_xy(x, y, __func__, __LINE__);

    int32_t ret = comb(sample, x, y, g_comb1)
        reverb_put(sample, ret);
    return ret;
}
#endif
