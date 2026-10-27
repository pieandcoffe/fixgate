//
// Created by Kyrylo Pylinskyi on 27/10/2026.
//

#include <fixgate/codec/tags.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <array>
#include <cstddef>


using fixgate::codec::Tag;
using fixgate::codec::to_number;

// compile time checks
static_assert(to_number(Tag::BeginString) == 8);
static_assert(to_number(Tag::MsgType) == 35);
static_assert(to_number(Tag::Symbol) == 55);
static_assert(to_number(Tag::CheckSum) == 10);

namespace {
    struct Expected {
        Tag tag;
        std::uint16_t number;
        const char* name;
    };

    constexpr std::array<Expected, 25> kExpected{{
        // Standard header
        {.tag = Tag::BeginString,       .number = 8,    .name = "BeginString"},
        {.tag = Tag::BodyLength,        .number = 9,    .name = "BodyLength"},
        {.tag = Tag::MsgType,           .number = 35,   .name = "MsgType"},
        {.tag = Tag::SenderCompID,      .number = 49,   .name = "SenderCompID"},
        {.tag = Tag::TargetCompID,      .number = 56,   .name = "TargetCompID"},
        {.tag = Tag::MsgSeqNum,         .number = 34,   .name = "MsgSeqNum"},
        {.tag = Tag::PossDupFlag,       .number = 43,   .name = "PossDupFlag"},
        {.tag = Tag::SendingTime,       .number = 52,   .name = "SendingTime"},
        {.tag = Tag::OrigSendingTime,   .number = 122,  .name = "OrigSendingTime"},
        // Order entry
        {.tag = Tag::ClOrdID,           .number = 11,   .name = "ClOrdID"},
        {.tag = Tag::Symbol,            .number = 55,   .name = "Symbol"},
        {.tag = Tag::Side,              .number = 54,   .name = "Side"},
        {.tag = Tag::TransactTime,      .number = 60,   .name = "TransactTime"},
        {.tag = Tag::OrderQty,          .number = 38,   .name = "OrderQty"},
        {.tag = Tag::OrdType,           .number = 40,   .name = "OrdType"},
        {.tag = Tag::Price,             .number = 44,   .name = "Price"},
        {.tag = Tag::TimeInForce,       .number = 59,   .name = "TimeInForce"},
        // Execution
        {.tag = Tag::OrderID,           .number = 37,   .name = "OrderID"},
        {.tag = Tag::ExecID,            .number = 17,   .name = "ExecID"},
        {.tag = Tag::ExecType,          .number = 150,  .name = "ExecType"},
        {.tag = Tag::OrdStatus,         .number = 39,   .name = "OrdStatus"},
        {.tag = Tag::LeavesQty,         .number = 151,  .name = "LeavesQty"},
        {.tag = Tag::CumQty,            .number = 14,   .name = "CumQty"},
        {.tag = Tag::AvgPx,             .number = 6,    .name = "AvgPx"},
        // Standard trailer
        {.tag = Tag::CheckSum,          .number = 10,   .name = "CheckSum"},
    }};
}

TEST(Tag, NumbersMatchFix44Specification) {
    for (const auto&[tag, number, name]: kExpected) {
        EXPECT_EQ(to_number(tag), number) << "wrong number for tag " << name;
    }
}

TEST(Tag, NumbersAreUnique) {
    for (std::size_t i = 0; i < kExpected.size(); ++i) {
        for (std::size_t j = i + 1; j < kExpected.size(); ++j) {
            EXPECT_NE(to_number(kExpected[i].tag), to_number(kExpected[j].tag))
                << kExpected[i].name << " and " << kExpected[j].name << " share a number";
        }
    }
}
