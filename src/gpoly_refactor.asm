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
	
	SORT_POLYPOINTS
	GET_POINT_DATA CalcShade, CalcText
	CALC_XVELS
	CALC_CREASE_LEN
	
	mov ecx, gpoly_mode
	jmp setupfortextshade
	
setupfortextshade:
	CALC_HSTEP CalcShade, CalcText
	CALC_DATA_EDGE_STEP CalcShade, CalcText, PixFixOff
	CALC_STARTPOS CalcShade, CalcText, PixFixOff
	PACK_DATA textshade
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
