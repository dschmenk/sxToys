/*
 * External memory routines.
 */
void xmemcpy(xdata byte *dst, xdata byte *src, byte count)
{
    _asm
        inc     _DPSEL                                  ; Switch to dptr1
        mov     dptr, #_AUTOPTRH
        mov     a, _xmemcpy_PARM_2+1
        movx    @dptr, a
        inc     dptr                                    ; _AUTOPTRL
        mov     a, _xmemcpy_PARM_2
        movx    @dptr, a
        inc     dptr
        mov     _MPAGE, dph                             ; _AUTODATA
        mov     r1, dpl
        inc     _DPSEL                                  ; Switch back to dptr0
        mov     a, _xmemcpy_PARM_3
        mov     r4, a
        jz      0002$
    0001$:
        movx    a, @r1
        movx    @dptr, a
        inc     dptr
        djnz    r4, 0001$
    0002$:
        mov     _MPAGE, r4
    _endasm;
    /*
     * Shut up compiler warnings.
     */
    dst=dst;
    src=src;
    count=count;
}
void xmemset(xdata byte *dst, byte val, byte count)
{
    _asm
        mov     r2, dpl
        mov     a, dph
        mov     dptr, #_AUTOPTRH
        movx    @dptr, a
        inc     dptr                                    ; _AUTOPTRL
        mov     a, r3
        movx    @dptr, a
        inc     dptr                                    ; _AUTODATA
        mov     a, _xmemset_PARM_3
        jz      0002$
        mov     r4, a
        mov     a, _xmemset_PARM_2
    0001$:
        movx    @dptr, a
        djnz    r4, 0001$
    0002$:
    _endasm;
    /*
     * Shut up compiler warnings.
     */
    dst=dst;
    val=val;
    count=count;
}

