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

  
  
  <em><strong>It is possible to resolve functions with IAT instead of EAT, smaller shellcode</em></strong>
  <br />Code in Resolve_Addr_From_IAT.c
