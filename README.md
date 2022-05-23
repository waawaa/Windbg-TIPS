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
<br /> <em><strong>bp 00411a6a "r @$t0 = @$t0 + 1; .if (@$t0 < 0n980) {gc} .else {.printf \"Reached\"; .echo }"</strong></em>
