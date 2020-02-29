.code32

.global start
start:
	pushl $11
	pushl $message1
	calll displayStr1 # print "(From Disk)"
	pushl $13
	pushl $message2
	calll displayStr2 # print "Hello, World!"
	addl $0x10, %esp # restore the previous esp state
	ret

message1:
	.string "(From Disk)"

message2:
	.string "Hello, World!"

displayStr1:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	movl $((80*6+13)*2), %edi
	movb $0xe0, %ah
nextChar1:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextChar1
	ret

displayStr2:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	movl $((80*6+0)*2), %edi
	movb $0x1a, %ah
nextChar2:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextChar2
	ret

