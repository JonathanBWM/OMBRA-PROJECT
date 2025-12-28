# December 2025 - Week 1
# Channel: #application-security
# Messages: 7

[2025-12-04 20:07] ml: Hi, do anyone know how to remove exception handler from ldrpInvertedFunctionTable?

[2025-12-04 20:08] the horse: RtlRemoveInvertedFunctionTable?

[2025-12-04 20:08] the horse: or RtlpRemoveInvertedFunctionTableEntry rather

[2025-12-04 20:08] the horse: ```cpp
__int64 __fastcall RtlpRemoveInvertedFunctionTableEntry(__int64 a1, int a2)
{
  int v2; // eax
  __int64 result; // rax

  _InterlockedIncrement(&dword_1801E8428);
  v2 = LdrpInvertedFunctionTables[0];
  if ( LdrpInvertedFunctionTables[0] != 2 )
  {
    memmove(
      &LdrpInvertedFunctionTables[4 * a2 + 4 + 2 * a2],
      &LdrpInvertedFunctionTables[4 * (a2 + 1) + 4 + 2 * (a2 + 1)],
      24LL * (unsigned int)(LdrpInvertedFunctionTables[0] - a2 - 1));
    v2 = LdrpInvertedFunctionTables[0];
  }
  result = (unsigned int)(v2 - 1);
  LdrpInvertedFunctionTables[0] = result;
  _InterlockedIncrement(&dword_1801E8428);
  return result;
}
```

[2025-12-04 20:09] ml: [replying to the horse: "RtlRemoveInvertedFunctionTable?"]
this one

[2025-12-04 20:10] the horse: yea that one will also adjust mrdata protect etc

[2025-12-04 20:13] ml: thx