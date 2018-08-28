// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//
// Events that go both ways
IPC_EVENT_TYPE0(DB_IPCE_INVALID_EVENT                ,0x0000)
IPC_EVENT_TYPE0(DB_IPCE_TYPE_MASK                    ,0x0FFF)

// Some rules:
// 1. Type0 is for marking sections in the id range.
//    Type1 is for events that go L->R, Type2 is for events that go R<-L.
// 2. All non-type 0 events should have a unique identifier & value
// 3. All type 1 events values should be in range [DB_IPCE_RUNTIME_FIRST, DB_IPCE_RUNTIME_LAST)
//    All type 2 events values should be in range [DB_IPCE_DEBUGGER_FIRST, DB_IPCE_DEBUGGER_LAST)
// 4. All event values should be monotonically increasing, though we can skip values.
// 5. All values should be a subset of the bits specified by DB_IPCE_TYPE_MASK.
//
// These rules are enforced by a bunch of compile time checks (C_ASSERT) in
// the function DoCompileTimeCheckOnDbgIpcEventTypes.
// If you get compiler errors in this file, you are probably violating the rules above.

// Events that travel from the RC to the DI (Left to Right)
IPC_EVENT_TYPE0(DB_IPCE_RUNTIME_FIRST                ,0x0100)   // change to TYPE0 because it is not really an event
IPC_EVENT_TYPE1(DB_IPCE_BREAKPOINT                   ,0x0100)
IPC_EVENT_TYPE1(DB_IPCE_SYNC_COMPLETE                ,0x0102)
IPC_EVENT_TYPE1(DB_IPCE_THREAD_ATTACH                ,0x0103)
IPC_EVENT_TYPE1(DB_IPCE_THREAD_DETACH                ,0x0104)
IPC_EVENT_TYPE1(DB_IPCE_LOAD_MODULE                  ,0x0105)
IPC_EVENT_TYPE1(DB_IPCE_UNLOAD_MODULE                ,0x0106)
IPC_EVENT_TYPE1(DB_IPCE_LOAD_CLASS                   ,0x0107)
IPC_EVENT_TYPE1(DB_IPCE_UNLOAD_CLASS                 ,0x0108)
IPC_EVENT_TYPE1(DB_IPCE_EXCEPTION                    ,0x0109)
IPC_EVENT_TYPE1(DB_IPCE_UNHANDLED_EXCEPTION          ,0x010A)
IPC_EVENT_TYPE1(DB_IPCE_BREAKPOINT_ADD_RESULT        ,0x010D)
IPC_EVENT_TYPE1(DB_IPCE_STEP_RESULT                  ,0x010E)
IPC_EVENT_TYPE1(DB_IPCE_STEP_COMPLETE                ,0x010F)
IPC_EVENT_TYPE1(DB_IPCE_BREAKPOINT_REMOVE_RESULT     ,0x0111)
IPC_EVENT_TYPE1(DB_IPCE_GET_BUFFER_RESULT            ,0x0115)
IPC_EVENT_TYPE1(DB_IPCE_RELEASE_BUFFER_RESULT        ,0x0116)
IPC_EVENT_TYPE1(DB_IPCE_ENC_ADD_FIELD                ,0x0117)
IPC_EVENT_TYPE1(DB_IPCE_APPLY_CHANGES_RESULT         ,0x0118)
IPC_EVENT_TYPE1(DB_IPCE_CUSTOM_NOTIFICATION          ,0x011B)
IPC_EVENT_TYPE1(DB_IPCE_USER_BREAKPOINT              ,0x011C)
IPC_EVENT_TYPE1(DB_IPCE_FIRST_LOG_MESSAGE            ,0x011D)
// DB_IPCE_CONTINUED_LOG_MESSAGE = 0x11E, used to be here in v1.1,
// But we've removed that remove the v2.0 protocol
IPC_EVENT_TYPE1(DB_IPCE_LOGSWITCH_SET_MESSAGE        ,0x011F)
IPC_EVENT_TYPE1(DB_IPCE_CREATE_APP_DOMAIN            ,0x0120)
IPC_EVENT_TYPE1(DB_IPCE_EXIT_APP_DOMAIN              ,0x0121)
IPC_EVENT_TYPE1(DB_IPCE_LOAD_ASSEMBLY                ,0x0122)
IPC_EVENT_TYPE1(DB_IPCE_UNLOAD_ASSEMBLY              ,0x0123)
IPC_EVENT_TYPE1(DB_IPCE_SET_DEBUG_STATE_RESULT       ,0x0124)
IPC_EVENT_TYPE1(DB_IPCE_FUNC_EVAL_SETUP_RESULT       ,0x0125)
IPC_EVENT_TYPE1(DB_IPCE_FUNC_EVAL_COMPLETE           ,0x0126)
IPC_EVENT_TYPE1(DB_IPCE_SET_REFERENCE_RESULT         ,0x0127)
IPC_EVENT_TYPE1(DB_IPCE_APP_DOMAIN_NAME_RESULT       ,0x0128)
IPC_EVENT_TYPE1(DB_IPCE_FUNC_EVAL_ABORT_RESULT       ,0x0129)
IPC_EVENT_TYPE1(DB_IPCE_NAME_CHANGE                  ,0x012a)
IPC_EVENT_TYPE1(DB_IPCE_UPDATE_MODULE_SYMS           ,0x012c)
IPC_EVENT_TYPE1(DB_IPCE_CONTROL_C_EVENT              ,0x012f)
IPC_EVENT_TYPE1(DB_IPCE_FUNC_EVAL_CLEANUP_RESULT     ,0x0130)
IPC_EVENT_TYPE1(DB_IPCE_ENC_REMAP                    ,0x0131)
IPC_EVENT_TYPE1(DB_IPCE_SET_VALUE_CLASS_RESULT       ,0x0133)
IPC_EVENT_TYPE1(DB_IPCE_BREAKPOINT_SET_ERROR         ,0x0134)
IPC_EVENT_TYPE1(DB_IPCE_ENC_UPDATE_FUNCTION          ,0x0137)
IPC_EVENT_TYPE1(DB_IPCE_SET_METHOD_JMC_STATUS_RESULT ,0x013a)
IPC_EVENT_TYPE1(DB_IPCE_GET_METHOD_JMC_STATUS_RESULT ,0x013b)
IPC_EVENT_TYPE1(DB_IPCE_SET_MODULE_JMC_STATUS_RESULT ,0x013c)
IPC_EVENT_TYPE1(DB_IPCE_GET_THREAD_FOR_TASKID_RESULT ,0x013d)
IPC_EVENT_TYPE1(DB_IPCE_CREATE_CONNECTION            ,0x0141)
IPC_EVENT_TYPE1(DB_IPCE_DESTROY_CONNECTION           ,0x0142)
IPC_EVENT_TYPE1(DB_IPCE_CHANGE_CONNECTION            ,0x0143)
IPC_EVENT_TYPE1(DB_IPCE_FUNC_EVAL_RUDE_ABORT_RESULT  ,0x0144)
IPC_EVENT_TYPE1(DB_IPCE_EXCEPTION_CALLBACK2          ,0x0147)
IPC_EVENT_TYPE1(DB_IPCE_EXCEPTION_UNWIND             ,0x0148)
IPC_EVENT_TYPE1(DB_IPCE_INTERCEPT_EXCEPTION_RESULT   ,0x0149)
IPC_EVENT_TYPE1(DB_IPCE_CREATE_HANDLE_RESULT         ,0x014A)
IPC_EVENT_TYPE1(DB_IPCE_INTERCEPT_EXCEPTION_COMPLETE ,0x014B)
IPC_EVENT_TYPE1(DB_IPCE_ENC_REMAP_COMPLETE           ,0x014C)
IPC_EVENT_TYPE1(DB_IPCE_CREATE_PROCESS               ,0x014D)
IPC_EVENT_TYPE1(DB_IPCE_ENC_ADD_FUNCTION             ,0x014E)
IPC_EVENT_TYPE1(DB_IPCE_GET_NGEN_COMPILER_FLAGS_RESULT,0x0151)
IPC_EVENT_TYPE1(DB_IPCE_SET_NGEN_COMPILER_FLAGS_RESULT,0x0152)
IPC_EVENT_TYPE1(DB_IPCE_MDA_NOTIFICATION             ,0x0156)
IPC_EVENT_TYPE1(DB_IPCE_GET_GCHANDLE_INFO_RESULT     ,0x0157)
IPC_EVENT_TYPE1(DB_IPCE_TEST_CRST                    ,0x0158)
IPC_EVENT_TYPE1(DB_IPCE_TEST_RWLOCK                  ,0x0159)
IPC_EVENT_TYPE1(DB_IPCE_LEFTSIDE_STARTUP             ,0x015C)
IPC_EVENT_TYPE1(DB_IPCE_METADATA_UPDATE              ,0x015D)
IPC_EVENT_TYPE1(DB_IPCE_RESOLVE_UPDATE_METADATA_1_RESULT,0x015E)
IPC_EVENT_TYPE1(DB_IPCE_RESOLVE_UPDATE_METADATA_2_RESULT,0x015F)
IPC_EVENT_TYPE1(DB_IPCE_DATA_BREAKPOINT              ,0x0160)
IPC_EVENT_TYPE1(DB_IPCE_BEFORE_GARBAGE_COLLECTION    ,0x0161)
IPC_EVENT_TYPE1(DB_IPCE_AFTER_GARBAGE_COLLECTION     ,0x0162)
IPC_EVENT_TYPE1(DB_IPCE_GET_CONTAINER_RESULT         ,0x0163)
IPC_EVENT_TYPE0(DB_IPCE_RUNTIME_LAST                 ,0x0164)   // The last event from runtime



// Events that travel from the DI to the RC (Right to Left)
IPC_EVENT_TYPE0(DB_IPCE_DEBUGGER_FIRST               ,0x0200)  // change to TYPE0 because it is not really an event
IPC_EVENT_TYPE2(DB_IPCE_ASYNC_BREAK                  ,0x0200)
IPC_EVENT_TYPE2(DB_IPCE_CONTINUE                     ,0x0201)
IPC_EVENT_TYPE2(DB_IPCE_LIST_THREADS                 ,0x0202)
IPC_EVENT_TYPE2(DB_IPCE_SET_IP                       ,0x0205)
IPC_EVENT_TYPE2(DB_IPCE_SUSPEND_THREAD               ,0x0206)
IPC_EVENT_TYPE2(DB_IPCE_RESUME_THREAD                ,0x0207)
IPC_EVENT_TYPE2(DB_IPCE_BREAKPOINT_ADD               ,0x0209)
IPC_EVENT_TYPE2(DB_IPCE_BREAKPOINT_REMOVE            ,0x020A)
IPC_EVENT_TYPE2(DB_IPCE_STEP_CANCEL                  ,0x020B)
IPC_EVENT_TYPE2(DB_IPCE_STEP                         ,0x020C)
IPC_EVENT_TYPE2(DB_IPCE_STEP_OUT                     ,0x020D)
IPC_EVENT_TYPE2(DB_IPCE_GET_BUFFER                   ,0x0211)
IPC_EVENT_TYPE2(DB_IPCE_RELEASE_BUFFER               ,0x0212)
IPC_EVENT_TYPE2(DB_IPCE_SET_CLASS_LOAD_FLAG          ,0x0217)
IPC_EVENT_TYPE2(DB_IPCE_CONTINUE_EXCEPTION           ,0x0219)
IPC_EVENT_TYPE2(DB_IPCE_ATTACHING                    ,0x021A)
IPC_EVENT_TYPE2(DB_IPCE_APPLY_CHANGES                ,0x021B)
IPC_EVENT_TYPE2(DB_IPCE_SET_NGEN_COMPILER_FLAGS      ,0x021F)
IPC_EVENT_TYPE2(DB_IPCE_GET_NGEN_COMPILER_FLAGS      ,0x0220)
IPC_EVENT_TYPE2(DB_IPCE_IS_TRANSITION_STUB           ,0x0221)
IPC_EVENT_TYPE2(DB_IPCE_IS_TRANSITION_STUB_RESULT    ,0x0222)
IPC_EVENT_TYPE2(DB_IPCE_MODIFY_LOGSWITCH             ,0x0223)
IPC_EVENT_TYPE2(DB_IPCE_ENABLE_LOG_MESSAGES          ,0x0224)
IPC_EVENT_TYPE2(DB_IPCE_FUNC_EVAL                    ,0x0225)
IPC_EVENT_TYPE2(DB_IPCE_SET_REFERENCE                ,0x0228)
IPC_EVENT_TYPE2(DB_IPCE_FUNC_EVAL_ABORT              ,0x022c)
IPC_EVENT_TYPE2(DB_IPCE_DETACH_FROM_PROCESS          ,0x022f)
IPC_EVENT_TYPE2(DB_IPCE_CONTROL_C_EVENT_RESULT       ,0x0230)
IPC_EVENT_TYPE2(DB_IPCE_FUNC_EVAL_CLEANUP            ,0x0231)
IPC_EVENT_TYPE2(DB_IPCE_SET_ALL_DEBUG_STATE          ,0x0232)
IPC_EVENT_TYPE2(DB_IPCE_SET_VALUE_CLASS              ,0x0234)
IPC_EVENT_TYPE2(DB_IPCE_SET_METHOD_JMC_STATUS        ,0x023a)
IPC_EVENT_TYPE2(DB_IPCE_GET_METHOD_JMC_STATUS        ,0x023b)
IPC_EVENT_TYPE2(DB_IPCE_SET_MODULE_JMC_STATUS        ,0x023c)
IPC_EVENT_TYPE2(DB_IPCE_GET_THREAD_FOR_TASKID        ,0x023d)
IPC_EVENT_TYPE2(DB_IPCE_FUNC_EVAL_RUDE_ABORT         ,0x0241)
IPC_EVENT_TYPE2(DB_IPCE_CREATE_HANDLE                ,0x0244)
IPC_EVENT_TYPE2(DB_IPCE_DISPOSE_HANDLE               ,0x0245)
IPC_EVENT_TYPE2(DB_IPCE_INTERCEPT_EXCEPTION          ,0x0246)
IPC_EVENT_TYPE2(DB_IPCE_DEBUGGER_INVALID             ,0x0249)   // An invalid event type
IPC_EVENT_TYPE2(DB_IPCE_GET_GCHANDLE_INFO            ,0x0251)
IPC_EVENT_TYPE2(DB_IPCE_RESOLVE_UPDATE_METADATA_1    ,0x0256)
IPC_EVENT_TYPE2(DB_IPCE_RESOLVE_UPDATE_METADATA_2    ,0x0257)
IPC_EVENT_TYPE2(DB_IPCE_GET_CONTAINER               , 0x0258)
IPC_EVENT_TYPE0(DB_IPCE_DEBUGGER_LAST                ,0x0259)   // The last event from the debugger

