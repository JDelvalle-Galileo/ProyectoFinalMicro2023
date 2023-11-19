		AREA |.text|, CODE, READONLY, ALIGN=2
		THUMB         
		REQUIRE8         
		PRESERVE8    

		EXPORT	registerDisplayAssembly
		EXPORT	registerModifyAssemblyR6
		EXPORT	registerModifyAssemblyR7
		EXPORT	registerModifyAssemblyR8
		EXPORT	registerModifyAssemblyR9
		EXPORT runAssembly
		EXPORT callAssembly
			
			
registerDisplayAssembly
	push{lr}
	
	str r0,   [r0]		;guarda el r0 en el primer slot
	str r1,   [r0,#4]	; guarda r1 en siguiente slot (4 bytes = 1 word)
	str r2,   [r0, #8]
    str r3,   [r0, #12]
    str r4,   [r0, #16]
    str r5,   [r0, #20]
    str r6,   [r0, #24]
    str r7,   [r0, #28]
    str r8,   [r0, #32]
    str r9,   [r0, #36]
    str r10,  [r0, #40]
    str r11,  [r0, #44]
    str r12,  [r0, #48]
    str SP,   [r0, #52]
    str LR,   [r0, #56]
	mov r5, pc
    str r5,   [r0, #60]
	
	pop{pc}
	

registerModifyAssemblyR6
	push{lr}
	
	mov r6, r0
	
	pop{pc}
	
	
	
registerModifyAssemblyR7
	push{lr}
	mov r7, r0
	pop{pc}
	
	
	
registerModifyAssemblyR8
	push{lr}
	
	mov r8, r0
	
	pop{pc}
	
	
	
registerModifyAssemblyR9
	push{lr}
	
	mov r9, r0
	
	pop{pc}
	
	
	
	
runAssembly
	orrs r0, r0, #1
	
	bx r0
	
	
	
	
callAssembly
	push{lr}
	
	blx r0
	
	pop{pc}
	
	

    
    end   ; End of file