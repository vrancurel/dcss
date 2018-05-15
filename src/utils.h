/*
 * Copyright 2017-2018 the QuadIron authors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __KAD_UTILS_H__
#define __KAD_UTILS_H__

#include <algorithm>
#include <cstdint>
#include <limits>
#include <random>
#include <string>

#include <easylogging++.h>

#include "exceptions.h"

namespace kad {

static inline uint32_t
stou32(std::string const& str, size_t* idx = nullptr, int base = 10)
{
    // NOLINTNEXTLINE(google-runtime-int)
    unsigned long result = std::stoul(str, idx, base);

    if (result > std::numeric_limits<uint32_t>::max()) {
        throw DomainError("stou32");
    }
    return static_cast<uint32_t>(result);
}

static inline uint64_t
stou64(std::string const& str, size_t* idx = nullptr, int base = 10)
{
    // NOLINTNEXTLINE(google-runtime-int)
    unsigned long long result = std::stoull(str, idx, base);

    if (result > std::numeric_limits<uint64_t>::max()) {
        throw DomainError("stou64");
    }
    return static_cast<uint64_t>(result);
}

// Return a reference to the global PRNG.
static inline std::mt19937& prng()
{
    static std::mt19937 PRNG;

    return PRNG;
}

/** Append at most `n` items from the start of `src` to `dst`. */
template <typename SrcContainer, typename DstContainer>
static inline void
safe_copy_n(const SrcContainer& src, size_t n, DstContainer& dst)
{
    std::copy_n(src.begin(), std::min(n, src.size()), std::back_inserter(dst));
}

// Logger for the simulator.
#define SIM_LOG_ID "simulator"
#define SIM_LOG(_level) CLOG(_level, SIM_LOG_ID)
#define SIM_VLOG(_level) CVLOG(_level, SIM_LOG_ID)

// Logger for the Ethereum interaction.
#define ETH_LOG_ID "ethereum"
#define ETH_LOG(_level) CLOG(_level, ETH_LOG_ID)
#define ETH_VLOG(_level) CVLOG(_level, ETH_LOG_ID)

} // namespace kad

#endif
