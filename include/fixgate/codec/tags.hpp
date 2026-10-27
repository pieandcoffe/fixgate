//
// Created by Kyrylo Pylinskyi on 27/10/2026.
//

#ifndef FIXGATE_CODEC_TAGS_HPP
#define FIXGATE_CODEC_TAGS_HPP

#include <cstdint>

namespace fixgate::codec {
    // The highest tag number FIX allows for user-defined tags is 39.999. UInt16 range is [0, 65.535]
    enum class Tag : std::uint16_t {
        // Standard header
        BeginString     = 8,
        BodyLength      = 9,
        MsgType         = 35,
        SenderCompID    = 49,
        TargetCompID    = 56,
        MsgSeqNum       = 34,
        PossDupFlag     = 43,
        SendingTime     = 52,
        OrigSendingTime = 122,

        // Order entry (first used in NewOrderSingle)
        ClOrdID         = 11,
        Symbol          = 55,
        Side            = 54,
        TransactTime    = 60,
        OrderQty        = 38,
        OrdType         = 40,
        Price           = 44,
        TimeInForce     = 59,

        // Execution (first used in ExecutionReport)
        OrderID         = 37,
        ExecID          = 17,
        ExecType        = 150,
        OrdStatus       = 39,
        LeavesQty       = 151,
        CumQty          = 14,
        AvgPx           = 6,

        // Standard trailer
        CheckSum        = 10,
    };

    constexpr std::uint16_t to_number(const Tag tag) noexcept {
        return static_cast<std::uint16_t>(tag);
    }
}
#endif //FIXGATE_CODEC_TAGS_HPP
