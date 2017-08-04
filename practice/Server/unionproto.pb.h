/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.8 at Mon Jul 31 15:56:31 2017. */

#ifndef PB_UNIONPROTO_PB_H_INCLUDED
#define PB_UNIONPROTO_PB_H_INCLUDED
#include "pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _MsgType3 {
    pb_callback_t playerName;
/* @@protoc_insertion_point(struct:MsgType3) */
} MsgType3;

typedef struct _MsgType1 {
    int32_t value;
/* @@protoc_insertion_point(struct:MsgType1) */
} MsgType1;

typedef struct _MsgType2 {
    bool value;
/* @@protoc_insertion_point(struct:MsgType2) */
} MsgType2;

typedef struct _UnionMessage {
    bool has_msg1;
    MsgType1 msg1;
    bool has_msg2;
    MsgType2 msg2;
    bool has_msg3;
    MsgType3 msg3;
/* @@protoc_insertion_point(struct:UnionMessage) */
} UnionMessage;

/* Default values for struct fields */

/* Initializer values for message structs */
#define MsgType1_init_default                    {0}
#define MsgType2_init_default                    {0}
#define MsgType3_init_default                    {{{NULL}, NULL}}
#define UnionMessage_init_default                {false, MsgType1_init_default, false, MsgType2_init_default, false, MsgType3_init_default}
#define MsgType1_init_zero                       {0}
#define MsgType2_init_zero                       {0}
#define MsgType3_init_zero                       {{{NULL}, NULL}}
#define UnionMessage_init_zero                   {false, MsgType1_init_zero, false, MsgType2_init_zero, false, MsgType3_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define MsgType3_playerName_tag                  1
#define MsgType1_value_tag                       1
#define MsgType2_value_tag                       1
#define UnionMessage_msg1_tag                    1
#define UnionMessage_msg2_tag                    2
#define UnionMessage_msg3_tag                    3

/* Struct field encoding specification for nanopb */
extern const pb_field_t MsgType1_fields[2];
extern const pb_field_t MsgType2_fields[2];
extern const pb_field_t MsgType3_fields[2];
extern const pb_field_t UnionMessage_fields[4];

/* Maximum encoded size of messages (where known) */
#define MsgType1_size                            11
#define MsgType2_size                            2
/* MsgType3_size depends on runtime parameters */
#define UnionMessage_size                        (23 + MsgType3_size)

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define UNIONPROTO_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
