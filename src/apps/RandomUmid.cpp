#include <assert.h>
#include <stdint.h>
#include <random>
#include <mutex>

#include <sys/time.h>
#include <sys/times.h>

#include <mxf/mxf_types.h>

static uint32_t generateRandom()
{
    static std::mutex rng_mutex;
    static std::mt19937 rng{std::random_device{}()};

    std::lock_guard<std::mutex> lock(rng_mutex);
    return rng();
}

static uint64_t generateMajorMinor(uint32_t offset)
{
    // the time() function returns the time in seconds since the epoch and times() function
    // returns the number of clock ticks since some arbitrary time instant.
    uint32_t major = (uint32_t)time(NULL);

    struct tms tms_buf;
    uint32_t minor = (uint32_t)times(&tms_buf);
    assert(minor != (uint32_t)-1);

    // Add an offset to the minor value for this process.
    // Each process should use a different random UMID generation offset.
    // This ensures that processes running on the same machine, which share the same
    // system time (times() above), will not produce the same minor value
    minor += offset;

    uint64_t parts = (static_cast<uint64_t>(major) << 32) | minor;

    // make sure that no (major, minor) pair is used twice
    static std::mutex last_mutex;
    static uint64_t last_parts = 0;

    std::lock_guard<std::mutex> lock(last_mutex);

    if (last_parts >= parts) {
        last_parts += 1;
    } else {
        last_parts = parts;
    }

    return last_parts;
}

// MobID generation code following the same algorithm as implemented in the AAF SDK
static mxfUMID generateUMID(uint32_t major, uint32_t minor)
{
    mxfUMID umid;
    umid.octet0 = 0x06;
    umid.octet1 = 0x0a;
    umid.octet2 = 0x2b;
    umid.octet3 = 0x34;
    umid.octet4 = 0x01;
    umid.octet5 = 0x01;
    umid.octet6 = 0x01;
    umid.octet7 = 0x01;
    umid.octet8 = 0x01;
    umid.octet9 = 0x01;
    umid.octet10 = 0x0f; // material type not identified
    umid.octet11 = 0x00; // no method specified for material and instance number generation
    umid.octet12 = 0x13;
    umid.octet13 = 0x00;
    umid.octet14 = 0x00;
    umid.octet15 = 0x00;

    umid.octet16 = static_cast<uint8_t>((major >> 24) & 0xff);
    umid.octet17 = static_cast<uint8_t>((major >> 16) & 0xff);
    umid.octet18 = static_cast<uint8_t>((major >>  8) & 0xff);
    umid.octet19 = static_cast<uint8_t>(major         & 0xff);

    umid.octet20 = static_cast<uint8_t>((minor >>  8) & 0xff);
    umid.octet21 = static_cast<uint8_t>(minor         & 0xff);
    umid.octet22 = static_cast<uint8_t>((minor >> 24) & 0xff);
    umid.octet23 = static_cast<uint8_t>((minor >> 16) & 0xff);

    umid.octet24 = 0x06;
    umid.octet25 = 0x0e;
    umid.octet26 = 0x2b;
    umid.octet27 = 0x34;
    umid.octet28 = 0x7f;
    umid.octet29 = 0x7f;
    umid.octet30 = static_cast<uint8_t>(42 & 0x7f);         // company specific prefix = 42
    umid.octet31 = static_cast<uint8_t>((42 >> 7L) | 0x80); // "
    return umid;
}

void bmx_generate_random_umid(mxfUMID *umid)
{
    static uint32_t offset = generateRandom();  // generate offset once for the process
    uint64_t parts = generateMajorMinor(offset);
    *umid = generateUMID(static_cast<uint32_t>(parts >> 32), static_cast<uint32_t>(parts));
}
