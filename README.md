# Windbg-TIPS

Break with conditions in a while loop, for example:
<br />
```
counter = 0
while (eax!='\0')
{
  ebx[counter] = ecx[counter]
  counter +=1 //We want to break when counter > 200
}
```

<br /> Break with pseudoregisters:
<br /> <em><strong>bp 00411a6a "r @$t0 = @$t0 + 1; .if (@$t0 < 0n200) {gc} .else {.printf \"Reached\"; .echo }"</strong></em>

  
  
  <h1>Shellcode IAT </h1>
<br /> It is possible to resolve functions with IAT instead of EAT, smaller shellcode</em></strong>
  <br />Code in Resolve_Addr_From_IAT.c

  
  
  <h1>Faronics reversing </h1>
<br />Exploit to trigger a heap overflow in faronics is availabale in crash_faronics.py</em></strong>
  <br />Post in [waawaa.github.io/](https://waawaa.github.io/es/reversing-faronics-deepfreeze/)
  
  
  
 <h1>Crazy Idea </h1>
<br />Do a ROP that instead of call VirtualAlloc would register a new SEH in the application, and when an exception is generated it redirects the flow to a handler, that really is our shellcode, which is registrated as a new handler</em></strong>

 
