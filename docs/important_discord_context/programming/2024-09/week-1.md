# September 2024 - Week 1
# Channel: #programming
# Messages: 3

[2024-09-01 17:48] hxm: <@162611465130475520>  how to add a subdir like 
`add_subdirectory(thirdparty/libs)`
[subdir.thirdparty] ==> how to precise path here

[2024-09-01 17:49] mrexodia: [replying to hxm: "<@162611465130475520>  how to add a subdir like 
`..."]
`[subdir."thirdparty/libs"]`

[2024-09-01 23:58] projizdivlak: can u call `NtCreateThread` without `Ex` in user mode? i always get `STATUS_ACCESS_DENIED`
```cpp
INITIAL_TEB InitialTeb;
NTSTATUS Status = RtlpCreateStack(ProcessHandle, StackReserve, StackCommit, StackZeroBits, &InitialTeb);

if (!NT_SUCCESS(Status))
{
    RtlpFreeStack(ProcessHandle, &InitialTeb);
    return Status;
}

CONTEXT ThreadContext;
Status = RtlInitializeContext(ProcessHandle, &ThreadContext, Parameter, StartAddress, InitialTeb.StackBase);

if (!NT_SUCCESS(Status))
{
    RtlpFreeStack(ProcessHandle, &InitialTeb);
    return Status;
}

BOOLEAN CreateSuspended = CreateFlags & THREAD_CREATE_FLAGS_CREATE_SUSPENDED;
HANDLE ThreadHandle;
CLIENT_ID ThreadCid;

Status = NtCreateThread(&ThreadHandle, THREAD_ALL_ACCESS, NULL, ProcessHandle, &ThreadCid, &ThreadContext, &InitialTeb, CreateSuspended);

if (!NT_SUCCESS(Status))
{
    RtlpFreeStack(ProcessHandle, &InitialTeb);
    return Status; /* STATUS_ACCESS_DENIED */
}
```