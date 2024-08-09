	CSEG SEGMENT PARA PUBLIC 'CODE'
	
	
	;params eax PolyPoint 1
	; edx PolyPoint 2
	; ebx PolyPoint 3
	; ecx PolyMode
	
	draw_gpoly_ PROC NEAR SYSCALL
	
	
	;-------- LOCAL_VARS MACRO
        xvel13 equ <(0 * 4)[esp]>
        xvel12 equ <(1 * 4)[esp]>
        xvel23 equ <(2 * 4)[esp]>
        
        shadeveltop equ <(3 * 4)[esp]>
        shadevelbottom equ <(4 * 4)[esp]>
        
        mapxvelbottom equ <(6 * 4)[esp]>
        mapxveltop equ <(7 * 4)[esp]>
        mapyvelbottom equ <(9 * 4)[esp]>
        mapyveltop equ <(10 * 4)[esp]>
        
        seccount equ <(12 * 4)[esp]>
        
        point1y equ <(13 * 4)[esp]>
        point1x equ <(14 * 4)[esp]>
        point1xfp equ <(15 * 4)[esp]>
        point1shade equ <(16 * 4)[esp]>
        point1mapx equ <(17 * 4)[esp]>
        point1mapy equ <(18 * 4)[esp]>
        
        point2y equ <(19 * 4)[esp]>
        point2x equ <(20 * 4)[esp]>
        point2xfp equ <(21 * 4)[esp]>
        point2shade equ <(22 * 4)[esp]>
        point2mapx equ <(23 * 4)[esp]>
        point2mapy equ <(24 * 4)[esp]>
        
        point3y equ <(25 * 4)[esp]>
        point3x equ <(26 * 4)[esp]>
        point3xfp equ <(27 * 4)[esp]>
        point3shade equ <(28 * 4)[esp]>
        point3mapx equ <(29 * 4)[esp]>
        point3mapy equ <(30 * 4)[esp]>
        
        creaselen equ <(31 * 4)[esp]>
        linewidth equ <(32 * 4)[esp]>
        
        startxvel equ <(33 * 4)[esp]>
        endxvel equ <(34 * 4)[esp]>
        shadevellow equ <(35 * 4)[esp]>
        shadevelhigh equ <(36 * 4)[esp]>
        shadehstephigh equ <(37 * 4)[esp]>
        shadehsteplow equ <(38 * 4)[esp]>
        
        scrwidth equ <(43 * 4)[esp]>
        
        seclen equ <(44 * 4)[esp]>
        startx equ <(45 * 4)[esp]>
        endx equ <(46 * 4)[esp]>
        lineaddr equ <(47 * 4)[esp]>
        shadelow equ <(48 * 4)[esp]>
        shadeaddr equ <(49 * 4)[esp]>
        
        mapaddr equ <(51 * 4)[esp]>
        mapxlow equ <(52 * 4)[esp]>
        mapylow equ <(54 * 4)[esp]>
        
        xcount equ <(55 * 4)[esp]>
        
        packedxvel1 equ <(57 * 4)[esp]>
        packedxvel3 equ <(58 * 4)[esp]>
        packedxvel2 equ <(59 * 4)[esp]>
        ycount equ <(60 * 4)[esp]>
        
        packedhstep1 equ <(61 * 4)[esp]>
        packedhstep2 equ <(62 * 4)[esp]>
        packedhstep3 equ <(63 * 4)[esp]>
        
        mapxhstep equ <(64 * 4)[esp]>
        mapyhstep equ <(65 * 4)[esp]>
        shadehstep equ <(66 * 4)[esp]>
        
        packedxvel1top equ <(67 * 4)[esp]>
        packedxvel2top equ <(68 * 4)[esp]>
        packedxvel3top equ <(69 * 4)[esp]>
        packedxvel1bottom equ <(70 * 4)[esp]>
        packedxvel2bottom equ <(71 * 4)[esp]>
        packedxvel3bottom equ <(72 * 4)[esp]>
        
        packedstartpos1top equ <(73 * 4)[esp]>
        packedstartpos2top equ <(74 * 4)[esp]>
        packedstartpos3top equ <(75 * 4)[esp]>
        packedstartpos1bottom equ <(76 * 4)[esp]>
        packedstartpos2bottom equ <(77 * 4)[esp]>
        packedstartpos3bottom equ <(78 * 4)[esp]>
        
        oldxp equ <(79 * 4)[esp]>
        
        packedxvel0top equ <(82 * 4)[esp]>
        packedxvel0bottom equ <(83 * 4)[esp]>
        
        packedxvel0 equ <(84 * 4)[esp]>
        packedhstep18bs equ <(85 * 4)[esp]>
        
        startposshadetop equ <(86 * 4)[esp]>
        startposmapxtop equ <(87 * 4)[esp]>
        startposmapytop equ <(88 * 4)[esp]>
        
        startposshadebottom equ <(89 * 4)[esp]>
        startposmapxbottom equ <(90 * 4)[esp]>
        startposmapybottom equ <(91 * 4)[esp]>
        
        needclip equ <(92 * 4)[esp]>
        
        shadevellow equ <(93 * 4)[esp]>
        shadevelhigh equ <(94 * 4)[esp]>
        
        packedouterloopdata0 equ <(95 * 4)[esp]>
        
        packedhstep0 equ <(96 * 4)[esp]>
        
        packedhstep28bs equ <(97 * 4)[esp]>
        packedhstep38bs equ <(98 * 4)[esp]>
        
        SPACE_FOR_VARS equ 100 * 4
    ;-------- END LOCAL_VARS MACRO

	;-------- PRAMDEF MACRO
        PixFixOn equ 1
        PixFixOff equ 0
        
        CalcShade equ 1
        NoCalcShade equ 0
        
        CalcText equ 1
        NoCalcText equ 0
        
        HClipOn equ 1
        HClipOff equ 0
    ;-------- END PRAMDEF MACRO
	
	pusha
	
	sub esp, SPACE_FOR_VARS
	
	add ecx, gpoly_pro_enable_mode_ofs
	mov gpoly_mode, ecx
	
handle:
	;-------- CLIP_CHECK MACRO
    	LOCAL clip, okx, oky
	
    assume eax:PTR PolyPoint
    assume edx:PTR PolyPoint
    assume ebx:PTR PolyPoint
        
        ;-------- Fast clip offscreen polys (off by default)
        if FAST_CLIP_OFF_SCREEN_POLYS
            mov ebp, [eax].X
            and ebp, [edx].X
            and ebp, [ebx].X
            js reject
            
            mov ebp, [eax].Y
            and ebp, [edx].Y
            and ebp, [ebx].Y
            js reject
            
            mov ebp, gpoly_clip_width
            cmp ebp, [eax].X
            jg okx
            cmp ebp, [edx].X
            jg okx
            cmp ebp, [ebx].X
            jle reject
            
        okx:
            mov ebp, gpoly_clip_height
            cmp ebp, [eax].Y
            jg oky
            cmp ebp, [edx].Y
            jg oky
            cmp ebp, [ebx].Y
            jle reject
            
        oky:
        endif
        ;-------- End fast clip offscreen polys
        
        ;-------- Back face culling (off by default)
        if BACK_FACE_CULL
            mov ebp, [edx].X
            sub ebp, [eax].X
            mov esi, [ebx].Y
            sub esi, [eax].Y
            imul esi, ebp
            mov ebp, [ebx].X
            sub ebp, [eax].X
            mov ecx, [edx].Y
            sub ecx, [eax].Y
            imul ecx, ebp
            sub ecx, esi
            jns reject
        endif
        ;-------- End back face culling
        
        mov ebp, [eax].X
        or ebp, [edx].X
        or ebp, [ebx].X
        js clip
        
        mov ecx, gpoly_clip_width
        mov esi, [eax].X
        cmp esi, ecx
        jg clip
        
        mov esi, [edx].X
        cmp esi, ecx
        jg clip
        
        mov esi, [ebx].X
        cmp esi, ecx
        jg clip
        
        mov ebp, 0
        mov needclip, ebp
        jmp calcend
        
    clip:
        mov ebp, 1
        mov needclip, ebp
        
    calcend:
    ;-------- END CLIP_CHECK MACRO
	
	;-------- SORT_POLYPOINTS MACRO
    	LOCAL dontswaptop1, dontswaptop2, dontswapmiddle
	
    assume eax:PTR PolyPoint
    assume edx:PTR PolyPoint
    assume ebx:PTR PolyPoint
        
        mov ecx, [eax].Y             ;Sort points
        cmp ecx, [edx].Y
        jle dontswaptop1
        mov ecx, [edx].Y
        xchg eax, edx
        
    dontswaptop1:
        cmp ecx, [ebx].Y
        jle dontswaptop2
        mov ecx, [ebx].Y
        xchg eax, ebx
        
    dontswaptop2:
        mov ecx, [edx].Y
        cmp ecx, [ebx].Y
        jle dontswapmiddle
        xchg edx, ebx
        
    dontswapmiddle:
        mov ecx, [eax].Y
        cmp ecx, [ebx].Y
        je reject
    ;-------- END SORT_POLYPOINTS MACRO

    ;-------- GET_POINT_DATA MACRO
	; Assume ShadeOn = 1 and TextOn = 1
    assume eax:PTR PolyPoint
    assume edx:PTR PolyPoint
    assume ebx:PTR PolyPoint
        
        mov ecx, [eax].X             ;load point data into point variables
        mov ebp, [eax].Y
        mov point1x, ecx
        mov point1y, ebp
        shl ecx, 16
        mov point1xfp, ecx
        
        mov ecx, [edx].X
        mov ebp, [edx].Y
        mov point2x, ecx
        mov point2y, ebp
        shl ecx, 16
        mov point2xfp, ecx
        
        mov ecx, [ebx].X
        mov ebp, [ebx].Y
        mov point3x, ecx
        mov point3y, ebp
        shl ecx, 16
        mov point3xfp, ecx
        
        ;-------- Shading on
        IF ShadeOn
            mov edi, [eax].Shade
            mov ebp, [edx].Shade
            mov ecx, [ebx].Shade
            shr edi, 16
            shr ebp, 16
            shr ecx, 16
            mov point1shade, edi
            mov point2shade, ebp
            mov point3shade, ecx
        ENDIF
        ;-------- End shading on
        
        ;-------- Texturing on
        IF TextOn
            mov edi, [eax].TMapX
            mov ebp, [eax].TMapY
            mov ecx, [edx].TMapX
            mov esi, [edx].TMapY
            shr edi, 16
            shr ebp, 16
            shr ecx, 16
            shr esi, 16
            mov point1mapx, edi
            mov point1mapy, ebp
            mov point2mapx, ecx
            mov point2mapy, esi
            mov ecx, [ebx].TMapX
            mov esi, [ebx].TMapY
            shr ecx, 16
            shr esi, 16
            mov point3mapx, ecx
            mov point3mapy, esi
        ENDIF
        ;-------- End texturing on
        
    assume eax:SLONG
    assume edx:SLONG
    assume ebx:SLONG
    ;-------- END GET_POINT_DATA MACRO

    ;-------- CALC_XVELS MACRO
    ; Calculates the vectors needed to step down each triangle edge
        LOCAL xv13calc, xv12calc, xv23calc
        LOCAL xvlarge13, xvlarge12, xvlarge23, calcend
        
        mov ecx, point3y             ;Calculate Xvel from points 1 to 3
        sub ecx, point1y
        je calcend
        
        mov eax, point3x
        sub eax, point1x
        
        test ecx, - 32
        jne xvlarge13
        
        cmp eax, - 32
        jl xvlarge13
        
        cmp eax, 31
        jg xvlarge13
        
        mov ebx, ecx                 ;use table for small triangle
        shl ebx, 8
        mov eax, [eax * 4 + ebx + _gpoly_divtable + 32 * 4]
        mov xvel13, eax
        
    xv12calc:                     ;calculate shade vel form point 1 to 3
        mov ecx, point2y             ;calculate for point 1 to 2
        sub ecx, point1y
        je xv23calc                  ;ignore if flat top
        
        mov eax, point2x
        sub eax, point1x
        test ecx, - 32
        jne xvlarge12
        
        cmp eax, - 32
        jl xvlarge12
        
        cmp eax, 31
        jg xvlarge12
        
        mov ebx, ecx
        shl ebx, 8
        mov eax, [eax * 4 + ebx + _gpoly_divtable + 32 * 4]
        mov xvel12, eax
        
    xv23calc:
        mov ecx, point3y             ;calculate for points 2 to 3
        sub ecx, point2y
        je calcend                   ;ignore if flat bottom
        
        mov eax, point3x
        sub eax, point2x
        test ecx, - 32
        jne xvlarge23
        
        cmp eax, - 32
        jl xvlarge23
        
        cmp eax, 31
        jg xvlarge23
        
        mov ebx, ecx
        shl ebx, 8
        mov eax, [eax * 4 + ebx + _gpoly_divtable + 32 * 4]
        mov xvel23, eax
        jmp calcend
        
    xvlarge13:
        shl eax, 16                  ;use division if triangle too big
        cdq
        idiv ecx
        mov xvel13, eax
        jmp xv12calc
        
    xvlarge12:
        shl eax, 16
        cdq
        idiv ecx
        mov xvel12, eax
        jmp xv23calc
        
    xvlarge23:
        shl eax, 16
        cdq
        idiv ecx
        mov xvel23, eax
        
    calcend:
    ;-------- END CALC_XVELS MACRO

    ;-------- CALC_CREASE_LEN MACRO
    ; Calculates the length of the widest point of the triangle
        mov ecx, point2y
        sub ecx, point1y
        mov ebp, xvel13
        mov esi, point1xfp
        imul ebp, ecx
        add esi, ebp
        sub esi, point2xfp
        mov creaselen, esi
    ;-------- END CALC_CREASE_LEN MACRO
	
	mov ecx, gpoly_mode
	jmp setupfortextshade
	
setupfortextshade:
    ;-------- CALC_HSTEP MACRO
    ; Calculates the vectors needed to step across the triangle
    ; returns vel variables
    	LOCAL calcend, posshade, posmapx, posmapy, zerocalc, bendonright
	
        mov esi, point2x
        mov edi, creaselen
        
        or edi, edi
        ;js bendonright
        
        sub esi, 2 * 0
        
        ;bendonright:
        add esi, 1 * 0
        mov eax, point1x             ;find denom cross product
        sub esi, eax
        mov edi, point3x
        sub edi, eax
        mov eax, point1y
        mov ebx, point2y
        sub ebx, eax
        mov ecx, point3y
        sub ecx, eax
        mov eax, ecx
        ;sar eax, 1
        imul ecx, esi
        mov ebp, creaselen
        or ebp, ebp
        js bendonright
        
        sub ecx, eax
        sub ecx, eax
        
    bendonright:
        add ecx, eax
        
        imul ebx, edi
        sub ebx, ecx
        je zerocalc
        
        xor edx, edx
        mov eax, 32768 * 65536 - 1
        idiv ebx
        mov ebp, eax
        mov eax, point1y
        mov esi, point3y
        sub esi, eax
        mov edi, point2y
        sub edi, eax
        
        IF ShadeOn
        mov eax, ebp
        mov edx, point1shade         ;calc x shade step
        mov ebx, point3shade
        sub ebx, edx
        mov ecx, point2shade
        sub ecx, edx
        imul ecx, esi
        imul ebx, edi
        sub ebx, ecx
        imul ebx
        shl eax, 1
        rcl edx, 1
        mov ax, dx
        rol eax, 16
        jns posshade
        
        inc eax
        
    posshade:
        mov shadehstep, eax
        ENDIF
        
        IF TextOn
        mov eax, ebp
        mov edx, point1mapx          ;calc x mapx step
        mov ebx, point3mapx
        sub ebx, edx
        mov ecx, point2mapx
        sub ecx, edx
        imul ecx, esi
        imul ebx, edi
        sub ebx, ecx
        imul ebx
        shl eax, 1
        rcl edx, 1
        mov ax, dx
        rol eax, 16
        jns posmapx
        
        inc eax
        
    posmapx:
        mov mapxhstep, eax
        mov eax, ebp
        mov edx, point1mapy          ;calc x mapy step
        mov ebx, point3mapy
        sub ebx, edx
        mov ecx, point2mapy
        sub ecx, edx
        imul ecx, esi
        imul ebx, edi
        sub ebx, ecx
        imul ebx
        shl eax, 1
        rcl edx, 1
        mov ax, dx
        rol eax, 16
        jns posmapy
        
        inc eax
        
    posmapy:
        mov mapyhstep, eax
        
        ENDIF
        
        jmp calcend
        
    zerocalc:
        xor eax, eax
        IF ShadeOn
        mov shadehstep, eax
        ENDIF
        
        IF TextOn
        mov mapxhstep, eax
        mov mapyhstep, eax
        ENDIF
        
    calcend:
    ;-------- END CALC_HSTEP MACRO

    ;-------- CALC_DATA_EDGE_STEP MACRO
	; CALC_DATA_EDGE_STEP CalcShade, CalcText, PixFixOff
    	LOCAL bendonright, calcend
	
        mov esi, creaselen
        or esi, esi
        js bendonright
        
        ; CALC_DATA_FOR_EDGE ShadeOn, TextOn, 1, 2, top
        CALC_DATA_FOR_EDGE MACRO ShadeOn, TextOn, pt1, pt2, sec
            LOCAL largey, smally, posshade, posmapx, posmapy
            
            mov ecx, point&pt2&y
            sub ecx, point&pt1&y
            cmp ecx, 255
            jg largey
            
            mov ebx, _gpoly_reptable[ecx * 4]
            jmp smally
            
        largey:
            mov edx, 0
            mov eax, 32768 * 65536 - 1
            idiv ecx
            mov ebx, eax
            
        smally:
            
            IF ShadeOn
            mov eax, point&pt2&shade
            sub eax, point&pt1&shade
            shl eax, 1
            imul ebx
            mov ax, dx
            rol eax, 16
            jns posshade
            
            inc eax
            
        posshade:
            mov shadevel&sec, eax
            
            ENDIF
            
            IF TextOn
            mov eax, point&pt2&mapx
            sub eax, point&pt1&mapx
            shl eax, 1
            imul ebx
            mov ax, dx
            rol eax, 16
            jns posmapx
            
            inc eax
            
        posmapx:
            mov mapxvel&sec, eax
            mov eax, point&pt2&mapy
            sub eax, point&pt1&mapy
            shl eax, 1
            imul ebx
            mov ax, dx
            rol eax, 16
            jns posmapy
            
            inc eax
            
        posmapy:
            mov mapyvel&sec, eax
            ENDIF
        ;-------- END CALC_DATA_FOR_EDGE MACRO

        ;CALC_DATA_FOR_EDGE ShadeOn, TextOn, 2, 3, bottom
        ; Code ommited - See above
        
        IF PixFix
        CALC_ERROR_FIX 12, top
        CALC_ERROR_FIX 23, bottom
        ENDIF
        
        jmp calcend
        
    bendonright:
        ; CALC_DATA_FOR_EDGE ShadeOn, TextOn, 1, 3, top
        
        IF PixFix
        CALC_ERROR_FIX 13, top
        ENDIF
        
    calcend:
    ;-------- END CALC_DATA_EDGE_STEP MACRO

	; CALC_STARTPOS CalcShade, CalcText, PixFixOff
    CALC_STARTPOS MACRO ShadeOn, TextOn, PixFix
	
	    ;CALC_STARTPOS_SEC ShadeOn, TextOn, 1, top, PixFix
        CALC_STARTPOS_SEC MACRO ShadeOn, TextOn, pt, sec, PixFix
            IF ShadeOn
                mov eax, point&pt&shade
                shl eax, 16
                
                IF PixFix
                    add eax, shadehstep
                ENDIF
                
                mov startposshade&sec, eax
            ENDIF
            
            IF TextOn
                mov eax, point&pt&mapx
                shl eax, 16
                
                IF PixFix
                    add eax, mapxhstep
                ENDIF
                
                mov startposmapx&sec, eax
                
                mov eax, point&pt&mapy
                shl eax, 16
                
                IF PixFix
                    add eax, mapyhstep
                ENDIF
                
                mov startposmapy&sec, eax
            ENDIF
        ;-------- END CALC_STARTPOS_SEC MACRO

	    ;CALC_STARTPOS_SEC ShadeOn, TextOn, 2, bottom, PixFix
        CALC_STARTPOS_SEC MACRO ShadeOn, TextOn, pt, sec, PixFix
            IF ShadeOn
                mov eax, point&pt&shade
                shl eax, 16
                
                IF PixFix
                    add eax, shadehstep
                ENDIF
                
                mov startposshade&sec, eax
            ENDIF
            
            IF TextOn
                mov eax, point&pt&mapx
                shl eax, 16
                
                IF PixFix
                    add eax, mapxhstep
                ENDIF
                
                mov startposmapx&sec, eax
                
                mov eax, point&pt&mapy
                shl eax, 16
                
                IF PixFix
                    add eax, mapyhstep
                ENDIF
                
                mov startposmapy&sec, eax
            ENDIF
        ;-------- END CALC_STARTPOS_SEC MACRO
	
	;-------- END CALC_STARTPOS MACRO

	;PACK_DATA textshade
    PACK_DATA MACRO packtype
        LOCAL calcend, bendonright
        
        ;PACK_HSTEP_&packtype
        PACK_HSTEP_textshade MACRO
            
            LOCAL posshade1, posmapx1, posshade2, posmapx2
            
            mov eax, mapxhstep
            mov edx, eax
            shl eax, 16
            sar edx, 16
            mov ax, shadehstep + 1
            or ax, ax
            jns posshade1
            
            sub eax, 65536
            sbb dl, 0
            
            
        posshade1:
            mov packedhstep1, eax
            mov packedhstep2, edx
            mov eax, mapyhstep
            shl eax, 16
            mov edx, mapyhstep
            sar edx, 16
            mov al, packedhstep2
            or al, al
            jns posmapx1
            
            sub eax, 256
            sbb dl, 0
            
        posmapx1:
            mov packedhstep2, eax
            mov packedhstep3, edx
            
            
            mov eax, mapxhstep           ;less accurate version for inner loop
            mov edx, eax
            shl eax, 16
            sar edx, 16
            mov ax, shadehstep + 1
            or ax, ax
            jns posshade2
            
            sub eax, 65536 - 1
            sbb dl, 0
            
        posshade2:
            mov packedhstep18bs, eax
            mov packedhstep28bs, edx
            mov eax, mapyhstep
            shl eax, 16
            mov edx, mapyhstep
            sar edx, 16
            mov al, packedhstep28bs
            or al, al
            jns posmapx2
            
            sub eax, 256
            sbb dl, 0
            
        posmapx2:
            mov packedhstep28bs, eax
            mov packedhstep38bs, edx
        ;-------- END PACK_HSTEP_textshade MACRO

        PACK_EDGE_&packtype top
        PACK_STARTPOS_&packtype top
        mov esi, creaselen
        or esi, esi
        js calcend
        
        ;PACK_EDGE_&packtype bottom
        PACK_EDGE_textshade MACRO sec	
            LOCAL posshade1, posmapx1
            
            mov eax, mapxvel&sec
            mov edx, eax
            shl eax, 16
            sar edx, 16
            mov ax, shadevel&sec + 1
            or ax, ax
            jns posshade1
            
            sub eax, 65536
            sbb dl, 0
            
        posshade1:
            mov packedxvel1&sec, eax
            mov packedxvel2&sec, edx
            mov eax, mapyvel&sec
            shl eax, 16
            mov edx, mapyvel&sec
            sar edx, 16
            mov al, packedxvel2&sec
            or al, al
            jns posmapx1
            
            sub eax, 256
            sbb dl, 0
            
            
        posmapx1:
            mov packedxvel2&sec, eax
            mov packedxvel3&sec, edx
        ;-------- END PACK_EDGE_textshade MACRO

        ;PACK_STARTPOS_&packtype bottom
        PACK_STARTPOS_textshade MACRO sec	
            mov ebx, startposshade&sec
            mov ecx, startposmapx&sec
            mov edx, startposmapy&sec
            
            mov packedstartpos3&sec, bl
            shr ebx, 8
            mov eax, ecx
            shl ecx, 16
            shr eax, 16
            mov cx, bx
            mov packedstartpos1&sec, ecx
            
            mov ecx, edx
            shl ecx, 16
            shl edx, 8
            shr edx, 24
            shl edx, 8
            mov cl, al
            mov packedstartpos2&sec, ecx
            mov dl, packedstartpos3&sec
            mov packedstartpos3&sec, edx
        ;-------- END PACK_STARTPOS_textshade MACRO
        
    calcend:
    ;-------- END PACK_DATA MACRO

	jmp setupmodeend
	
setupmodeend:
	mov ecx, gpoly_mode
	jmp modetextshade
	
modetextshade:
	IF UseTextureGShade
	POLY_RENDER textshade, CalcShade, CalcText, PixFixOff, textshade, textshade, textshade, jtscan, textshade
	ENDIF
	
	add esp, SPACE_FOR_VARS
	popa
	ret
