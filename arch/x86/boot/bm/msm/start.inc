;-----------------------------------------------------
; File: msm\start.inc 
;
; Descri��o: 
;     In�cio do modo protegido. 
;     O modo protegido j� foi configurado e come�am aqui as rotinas em 32bit. 
;
; Atribui��es: 
;     +Efetivar a escolha do modo de inicializa��o.
;     +Passar o comando para Shell do Boot Manager
;      se essa for a escolha.
;     +Carregar o Boot Loader e passar o comando 
;      pra ele se essa for a escolha.
;
; Op��es de inicializa��o:
; ========================
; +1 - Inicia o Shell do Boot Manager no modo texto.
; +2 - Inicia o Boot Loader (BL.BIN) no modo GR�FICO. (GUI).
;
; Contexto dos registradores:
; ===========================
; Segments:
;    DS  =  stage2_DATA_SEL  
;    ES  =  stage2_DATA_SEL
; Stack:
;    Segment  =  stage2_DATA_SEL
;    Offset   =  0x0009FFFC  ?? @todo: Cuidado.
; Registers:
;    AL  = new initialization mode.
;    EBX = 0
;    ECX = 0
;	 EDX = 0
;
; Vers�o: 1.0, 2015, 2016.
;


;---------------------------------------
; start:
;     In�cio do modo protegido.
; IN:
;     AL = New mode.  (o modo a ser inicializado)
;
; Pega o modo de v�deo passado pelo stage 2 atrav�s de AL e salva no contexto 
; de 32 bits.
;
start:	
    
	;; #bugbug Esse valor pode estar errado durante 
	;; a inicializa��o em modo texto.
	mov byte [BOOT.modo], al			    
    
	xor eax, eax	
    xor ebx, ebx	
    xor ecx, ecx	
    xor edx, edx    ;Voice,8.	
;	
; ajusta_modo: 
;     Inicia um modo da configura��o permanente.	
;     @todo: Trocar o nome.
;
ajusta_modo:
    nop
startSetupMode:

	;Pega o modo de inicialisa��o do Boot Manager. 
	xor eax, eax

	;; #bugbug Esse valor pode estar errado durante 
	;; a inicializa��o em modo texto.
	mov al, byte [BOOT.modo]  
	
	;
	; Escolhe o modo:
	; ==============
	;
	; modo 1 - Inicia o BL.BIN no modo TEXTO.(Default).
	; modo 2 - Inicia o BL.BIN no modo GR�FICO.
	; modo 3 - Inicia a prompt de comandos do bootmanager.
	; modo 4 - (reservado)
	; Default.
	;
	
	cmp al, byte 1  
	je startCLI
	
	cmp al, byte 2  
	je startGUI	
	
	
	;;
	;; ## Corre��o de problemas ##
	;;
	
	
	;; Se 'al' estiver zerado entraremos em modo texto.
	;; O modo texto ser� o padr�o.
	
.default:	
	jmp startCLI 
	;jmp $
	

;==============================================================
; startCLI:	
;     Inicia o Shell do Boot Manager. 
;	
startCLI:
	
	call setup_machine
	
	cli
	;Segmentos.
	mov ax, word 0x10  
	mov ds, ax		 
	mov es, ax 
	;Pilha.
	mov eax, stack_ptr
	mov esp, eax 
	
	;Configura volume.
	mov al, byte 1
	call set_fs_volume  ;;?? onde ?? blconfig.inc
	
	;Configura fs type.
	mov al, byte 1
	call set_fs_type
	
	;Limpa a tela em modo texto.
	call clear_screen
	
    ;Habilita todas interrup��es.
.enableInterrupts:	
	mov  al, 0					                  
	out  0xA1, al                                   
	out  0x21, al    
	sti 
	
    ;;
    ;;  ##  SHELL LOOP  ##
    ;;    

	;; #importante
	;; O shell est� em msm/shell
	
	
;; Esse � loop do shell do boot manager.	
startShellLoop:	
    
	;;shell.inc
	call shellPrintShellVersion    
	
.shellInit:	
    
	;;shell.inc
	call shellInit                
    
	; Sairemos do loop.
    ; 0 - Nenhuma altera��o foi feita, 
	; n�o precisa salvar o metafile.
	cmp eax, 0
	je .exit0

	; Sairemos do loop.
    ; 1 - Alguma altera��o foi feita, 
	; precisa salvar o metafile.
	cmp eax, 1
	je .exit1
	
	jmp startShellLoop	
	JMP $

;	
;Saimos do loop do shell.
;Nenhuma altera��o foi feita, n�o precisa salvar o metafile.
;Apenas reiniciamos o sistema.
.exit0:
	mov esi, start_msg_rebooting
	call os_output	
    jmp keyboardReboot
	jmp $

;	
;Saimos do loop do shell.
;Alguma altera��o foi feita, precisamos salvar o metafile.
;Reiniciamos o sistema para validar as altera��es feitas no metafile. 
.exit1:
	mov esi, start_msg_saving_metafile
	call os_output	
	;obs: Essa rotina deve ser realizada em 32bit.
    call fs32SaveMetafile ;@todo (fs\fs32.inc) 
	
	mov esi, start_msg_rebooting
	call os_output		
	jmp keyboardReboot
	jmp $	
	
	
start_msg_saving_metafile  db 'start: Saving metafile..',13,10,0
start_msg_rebooting        db 'start: Rebooting..',13,10,0
	
	
;==========================================================================
; startGUI:
;     Inicia o BL.BIN no modo gr�fico.
;	  Obs: Quando retornamos para o modo real e ativamos o modo gr�fico,
; satalmos para c� com a op��o de usamos mensagens em modo gr�fico em modo protegido.
;
;
;; * IMPORTANTE:
;;   O OBJETIVO � CHEGARMOS AQUI COM O ARQUIVO BL.BIN CARREGADO NA MEM�RIA
;;   E PASSARMOS O COMANDO PARA ELE, SEM TERMOS QUE CARREGA LO USANDO RECURSOS DE 32BIT.
;
;
startGUI:
	
	;;
	;; *Importante:
	;; Fazendo configura��es aqui mesmo ao inv�s de chamarmos 
	;; 'setup_machine' ou ainda n�o fazendo essas configura��es, 
	;; pois j� estamos em modo protegido com as interrup��es 
	;; desabilitadas e em modo gr�fico ..
	;; Podemos deixar que o boot loader fa�a suas inicializa��es. 
	;; Obs: Isso est� funcionando bem.
	;; Mas o timer foi configurado v�rias vezes.
	;;	
	
	;;===============
	;;setup machine
	
	
	;GDT and IDT. 
	cli
	lgdt [lgdt_opcode] 
	lidt [lidt_opcode]	
	
	
	;;
	;; @todo: Nessa hora alguns vetores podem ser configurados.
	;;        primeiro as excess�es e depois as ISRs.
	;;

	;;
	;; Selecting the 'Processor Interrup Mode'.
	;; * PIC MODE *
	;;
		
	;;
    ;; Todos os componentes APIC s�o ignorados e o sistema opera
    ;; no modo single-thread usando LINT0.
    ;;	
		
;.setupPICMODE:	

	;PIC.
	cli
	mov al, 00010001b		;begin PIC 1 initialization.
	out 0x20, al
	mov al, 00010001b		;begin PIC 2 initialization.
	out 0xA0, al	
	mov al, 0x20			;IRQ 0-7: interrupts 20h-27h.
	out 0x21, al
	mov al, 0x28			;IRQ 8-15: interrupts 28h-2Fh.
	out 0xA1, al
	mov al, 4
	out 0x21, al
	mov al, 2
	out 0xA1, al
	mov al, 1
	out 0x21, al
	out 0xA1, al
	
	
	;;
	;; Configurando os timers do sistema.
	;; Nossa op��o agora � o PIT.
	;; @todo: Para o RTC podemos fazer uma pequena inicializa��o agora.
	;; pois um m�dulo mais completo est� no kernel base.
	;; #importante: #bugbug
	;; Talvez seja um erro configurarmos o PIT agora,
	;; Pois todas essas interrup��es podem 
	;; prejudicar o carregamento no boot loader.
	;; Obs: Testando sem essa configura��o.
	;;
	
	;TIMER.
	;mov al, byte 0x36
	;mov dx, word 0x43
	;out dx, al
	;mov eax, dword 11930    ;Timer frequency 100 HZ.
	;mov dx, word 0x40
	;out dx, al
	;mov al, ah
	;out dx, al
	
		
	
	;;@todo:
	;;Init RTC.

	
	;;step 
    ;;@todo: memory caching control.	
	
	;;step 
	;processor Discovery and initialization
	;(cpuid threads and cores, start inter-processor interrupt sipi, ap wakeu pstate,
	;wakeup vector alignment, caching consdarations,ap idle state)	
	;apenas o b�sico para o boot manager.
	
		
	;;
	;; Fazendo alguma inicializa��o de dispositivos de I/O suportados.
	;;

	;step
	;i/o devices
	;( embedded controller EC, super io SIO, legacy free systems, miscellaneous io devices)
    ;Nesse momento os dispositivos de i/o suportados aqui s�o (keyboard, IDE, graphic card).
	
	;KEYBOARD.
	call keyboardInit		
	
	
	;IDE0.   #BUGUBG NO QEMU
	;;call init_ide0_master
	
		
	
	;LFB.   #BUGUBG NO QEMU
	;;call configura_lfb   ;;?? onde est� essa rotina ??
	
	

	
	;
	; #BUGBUG
	; @todo:
	;     Aqui estamos usando constantes, porem esses valores precisam ser
	; detectados, pois variam � cada formata��o.
	;
	
	;Detectar os par�metros do sistema de arquivos FAT16.
	;fat lba, root lba ...
	;call fat16DetectFSParameters  ;;fat16lib.inc
	
	;Setup current dir.
    mov dword [g_current_fat_lba],        FAT16_FAT_LBA       ;Fat do volume atual
    mov dword [g_current_directory_lba],  FAT16_ROOTDIR_LBA   ;Diret�rio do volume atual.
	mov dword [g_current_dataarea_lba],   FAT16_DATAAREA_LBA  ;?? ;lba do inicio da area de dados do volume atual.
	;Continua...
		
	

	
    ;;
    ;; Configurando PCI. (O b�sico suportado pelo Boot Manager.)
	;; O boot manager est� carregando o BL utilizando recursos 
	;; do BIOS em modo real. Mas se o BM fosse carregar o BL 
	;; de um dispositivo SATA em 32bit teria que configurar o PCI.
	;; (Se poss�vel deixaremos para o BL o penoso trabalho de lidar
	;; com PCI, j� que o BL � feito em C, isso deve ser mais f�cil para
	;; o BL do que para o BM.)
    ;;
	
	;step
	;;PCI device descovery.	

	
	

	;CURSOR.
	call reset_cursor 
	

	;
	; Continua ...
	;
	
	cli 

    ;Mascara todas as interrup��es.	
	mov  al, 255
	out  0xA1, al
	out  0x21, al	
	
	
	
	;;=============================================================================
	
	
	
	
	
	
	
	cli
	;Segmentos.
	mov ax, word 0x10  
	mov ds, ax		 
	mov es, ax 
	;Pilha.
	mov eax, stack_ptr
	mov esp, eax 
			
	;;
    ;;  ### bugbug ##   Essa rotina carrega o MBR pera pegar informa��es da parti��o, possivelmente
	;; carregando o mbr em algum lugar que n�o devia...
    ;;
	
	;Pega parametros da parti��o ativa.		
    ;;##bugbug supensa por sujar areas importantes.
	;call pegar_parametros_da_particao_ativa	  ;;blconfig.inc
	
	;Configura volume.
	mov al, byte 1
	call set_fs_volume
	
	;Configura fs type.
	mov al, byte 1
	call set_fs_type
	
	;;internas.
    ;Configura LFB e Buffer.
	call configura_lfb
    call configura_buffer
	
	
	;; #bugbug
	;; ?? Porque habilitar isso agora ??		
	;mov  al, 0					                  
	;out  0xA1, al                                   
	;out  0x21, al    
	;sti 
	
	;
	;se configurar o modo 3 n�o tem como 
	;reconfigurar atravez desse modo ainda.
	;entao salva modo 1
	;	
	;mov eax, dword 1
	;call blconfig_set_modo	
	;call do_save
	
	;@todo: N�o � certo pintar um backgraund direto na mem�ria de video.
    ;o Boot Manger n�o tem sincroniza��o vertical ainda.	 
	 
	;background test!
    ;call _background
	;call _background_teste       
	;call _fade_screen
	;jmp $
	
	
	;;
	;; **** TESTING STRINGS ****
	;;
	
	;;
	;; bug bug as rotinas de 'char' 'string' e 'load' aparentemente
	;; est�o travando o sistema, as duas janelas de status est�o funcionando
	;; quando n�o chamamos essas fun��es.
	;;
	
	
	;
	; * Aqui vamos testar a rotina de mostrar strings no modo gr�fico.
	;   O caractere deve ser constru�do, p�xel por p�xel, utilizando
	; o mapa de caracteres do BIOS.
	;
	
.testingStrings:
    nop
	;@todo: Testar rotina de mostrar uma string.
    
	
	


	;debug break
	;jmp $
	
	;Debug:
	;Pinta uma janela pra testar modo gr�fico.
	
	;mov eax, 0    ;x.
	;mov ebx, 0    ;y.
	;mov ecx, 20   ;largura.
	;mov edx, 20   ;altura.	
    ;mov edi, dword  0x80808000    ;blue    	
	;call CreateWindow
	;call _asm_refresh_screen
	
  
  
  ;;
  ;; ## BUG BUG
  ;;
  ;; O teste indicou que as rotinas de char e string est�o
  ;; com mal funcionamento, travando o sistema.
  ;; a segunda janale n�o aparece que sinaliza prograsso n�o aparece.
  ;;
  
  
  ;;
  ;; @todo: Fazer debug da fun��o print char antes de tudo ...
  ;;
  ;; status: testando ...
  ;;
  
;Char.
    ;
    ; a =  cor (rrggbbaa)
    ; b =  x
    ; c =  y
    ; d =  Char.
    ;	    
  
.TestingPrintChar:  
	
	;push eax
	;push ebx
	;push ecx 
	;push edx 
	;mov eax, 0x00ffff00  ;; Color
	;xor edx, edx 
	;mov dl, 'a'          ;; Caractere de teste.	
	;call lib32PrintChar	
	;pop edx 
	;pop ecx 
	;pop ebx 
	;pop eax 
	;call _asm_refresh_screen
	
	
	;push eax
	;push ebx
	;push ecx 
	;push edx 	
	;mov eax, 0x00ffff00  ;; Color
	;xor edx, edx 
	;mov dl, 'b'          ;; Caractere de teste.	
	;call lib32PrintChar	
	;pop edx 
	;pop ecx 
	;pop ebx 
	;pop eax 
    ;call _asm_refresh_screen


	;push eax
	;push ebx
	;push ecx 
	;push edx 
	;mov eax, 0x00ffff00  ;; Color
	;xor edx, edx 
	;mov dl, 'A'          ;; Caractere de teste.	
	;call lib32PrintChar	
	;pop edx 
	;pop ecx 
	;pop ebx 
	;pop eax 
    ;call _asm_refresh_screen

	;push eax
	;push ebx
	;push ecx 
	;push edx 
	;mov eax, 0x00ffff00  ;; Color
	;xor edx, edx 
	;mov dl, 'B'          ;; Caractere de teste.	
	;call lib32PrintChar	
	;pop edx 
	;pop ecx 
	;pop ebx 
	;pop eax 	
	;call _asm_refresh_screen
	
	;debug break
	;jmp $

.TestingPrintString:	
	
	;;Cursor.
	mov dword [g_cursor_x], 4
	mov dword [g_cursor_y], 4		
	
	;String
	;mov esi, msg_start_loading
	;call lib32PrintString
	;call _asm_refresh_screen
	
	
  	;;debug
	mov esi, startMsgTest1   ;;crazy message
	call lib32PrintString
    call _asm_refresh_screen
	;;jmp $	
	
	;
	; @todo: testando modos no geforce.
	;
    ;jmp $
	
    ;
	; @todo: 
	;     N�o h� fun��es de string em modo gr�fico no bootmanager ainda.
	;     Ent�o para o carregamento do BL.BIN n�o h� mensagens.      
	;

	
	;
	; Carrega o Boot Loader.
	; Carrega o arquivo  BL.BIN na mem�ria e passa o comando para ele.
	; Enviando argumento em AL, e EBX.
	;
.LoadBootLoader:	
	
    ;;;;
	;;;;    ****  ATEN��O   ****
	;;;;
	;;;; O BL.BIN FOI CARREGADO PELO MBR USANDO RECURSOS DO BIOS.
	;;;;  *N�O* DEVEMOS CARREGA-LO NOVAMENTE NESSE MOMENTO USANDO RECURSO DE 32BIT.
	;;;;
	
	;####CANCELADO####
	;call load_boot_loader
	
	;
	; Obs: N�o carrega mais pois ele est� em assembly 
	; e faz parte do System4Nora.
	;
	
	;
	; Obs: Estamos sinalizando o progresso desenhando janelas,
	;      m�s haver� uma rotina de imprime caracteres.
	;
	
	
	;mov eax, 40  ;x.
	;mov ebx, 0   ;y.
	;mov ecx, 20  ;largura.
	;mov edx, 20  ;altura.	
    ;mov edi, dword  0x80808000    ;blue    	
	;call CreateWindow
	;call _asm_refresh_screen

	mov esi, msg_start_ok
	call lib32PrintString	
	call _asm_refresh_screen
	
	
;;
;;======================
;;
;.StartBootLoader:

	mov esi, msg_start_jumping
	call lib32PrintString
	call _asm_refresh_screen
    ;jmp $
	
    ;cabe�alho PE. no BL.BIN.
	mov edx, dword 0x20000 ;IMAGE_ADDRESS
	mov eax, [edx]
    
	;Primeiro byte 
	cmp al, byte 'L'
	jne .peFail	
	
	inc edx 
	mov eax, [edx]

	;segundo byte
	cmp al, byte 0x01
	jne .peFail	
	
	
.preparandoArgumentos:
 
    ;;
	;; *Boot Block - A tabela est� sendo preparada.
	;;
	
	;Prepara a tabela.
	mov ebp, dword BootBlock
	
	xor eax, eax
	mov eax, dword [ModeInfo_PhysBasePtr]  ;LFB address.	
	mov dword [BootBlock.lfb], eax	
	
	xor eax, eax
	mov ax, word [ModeInfo_XResolution]    ;Width in pixels.
	mov dword [BootBlock.x], eax 	
	
	xor eax, eax
	mov ax, word [ModeInfo_YResolution]    ;Height in pixel.
	mov dword [BootBlock.y], eax 	
	
	xor eax, eax
	mov al, byte [ModeInfo_BitsPerPixel]   ;BPP.
	mov dword [BootBlock.bpp], eax 

	xor eax, eax
	mov eax, dword META$FILE                 ;metafile address	
	mov dword [BootBlock.metafile_address], eax	
    
	;Continua...
	
	;xor eax, eax
	mov al, byte 'G'                       ;Flag. (useing graphics)	
	mov ebx, dword [ModeInfo_PhysBasePtr]  ;LFB address (endere�o f�sico)
	mov ecx, ebp                           ;tabela
	mov edx, ebp                           ;tabela
	
	;
    ; Go! ( Passa o comando para o entrypoint do Boot Loader. ) BL.BIN. 
    ;	
	
	;call _background_teste
	;call _background
	;call _fade_screen
	;jmp $
	
	
	;;
	;; A ASSINATURA FALHO ... � SINAL QUE O BOOTOLADER N�O FOI CARREGADO CORRETAMENTE,
	;; OU N�O FOI CARREGADO NO LOCAL CERTO...
	;; DEREPENTE UM PEQUENO AJUSTE DE ENDERE�O NO MBR RESOLVE O PROBLEMA.
	;;
	;;
	

	
	;debug
	;jmp $
	
	;;
	;; @todo: refazer os argumentos ... o boot block percisa ser preexido corretamente
	;; para isso o MBR precisa passar argumentos ...isso tem que refazer.
	;;
.goBL:
	
	jmp 8:IMAGE_ADDRESS_ENTRY_POINT
	JMP $
	
;O arquivo n�o est� presente na mem�ria.	
.peFail:
    mov eax, 0x00ffff00 ;color
	mov esi, msg_start_sig
	call lib32PrintString
    call _asm_refresh_screen
	
    mov eax, 0x00ffff00 ;color
	mov esi, 0x20000
	call lib32PrintString
    call _asm_refresh_screen
	
    jmp $

		
	;
	; @fim, Come�am as rotinas internas do arquivo.
	;
	
;
; Libs --------------------------------------------
;


;-------------------------------------
; configura_lfb:
;     Atualiza LFB address.
;     Salva LFB.
;++

;startSetupLFB:
	
configura_lfb:	 
	push edx   
	mov  edx, dword [ModeInfo_PhysBasePtr]    	
	mov dword [VIDEO.lfb], edx	
	pop edx
	ret
;--

	
;-------------------------------------
; configura_buffer:
;     Configura buffer para modo gr�fico.
;     
;     @todo: Confirmar os endere�os dos buffer.
;            Configurar p�ginas para eles.  
;
;++

;startSetupBuffer:
	
configura_buffer:
	;buffer 1,2 e 3.
	push edx
   	mov edx, dword 0x01000000		
	mov dword [VIDEO.buffer1], edx
   	mov edx, dword 0x01500000		
	mov dword [VIDEO.buffer2], edx
   	mov edx, dword 0x01A00000		
	mov dword [VIDEO.buffer3], edx 
	pop edx
	ret
;--	
	
	
;
; Start mensagens.
;

;usando em start.
startMsgTest1        db "BM:startGUI: startGUI: :)", 13,10,0
msg_start_ok         db "BM:startGUI: OK", 13,10,0	
msg_start_jumping    db "BM:startGUI: Jumping to BL.BIN..",13,10,0		
msg_start_sig        db "BM:startGUI: PE sig fail!",13,10,0	

;filtrar.
;msg_start_debug_hang db "BM: *start: Debug hang!", 13,10,0	
;msg_start1           db "BM: Start1", 13,10,0	
;msg_start2           db "BM: Start2", 13,10,0
;msg_start3           db "BM: Start3", 13,10,0
;msg_start4           db "BM: Start4", 13,10,0	
;msg_start_loading    db "BM: Loading BL.BIN..", 13,10,0
;msg_no_new_mode_todo db "BM: @todo: save new mode in a file.", 13,10,0	
;msg_no_new_mode      db "BM error: video mode fail!", 13,10,0	
;msg_iniciando_shell  db "BM: Starting Boot Manager Shell..", 13,10,0


;...

;
; End.
;