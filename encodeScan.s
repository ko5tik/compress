   # register definitions
#define pixPtr $4
#define scanWidth $5
#define scanHeight $6
#define outP $7
#define huffP $8

#define outPsave $9

#define pred $10

#define predFirst $11

#define pix2do $12
#define row2do $6

#define work $14

#define pixel $15

#define diff $18
  # thoe will be loaded together
#define huffCode $16
#define huffSize $17

#define huffBits $19
#define huffBitsFree $20

#define zero $0

	.verstamp	7 20
	.option	pic2
	.text	
	.align	2

	.globl	encodeScan


	.ent	encodeScan 2
encodeScan:
	.option	O1
	.set	 noreorder
	.cpload	$25
	.set	 reorder
	.frame	$sp, 0, $31

 #   get huffman pointer into register
	lw	huffP, 16($sp)

 # here starts the stuff
	move outPsave , outP
	li huffBitsFree , 32
	move huffBits , zero

startRow:
firstPix:
  #load first pixel
	lbu pixel , (pixPtr)
  # init predictor for this
	li pred , 0x80
restartRow:
  # setup predictor for the first pixel of next row
	move predFirst, pixel
  # setup amount of pixel to do in this row
        move pix2do , scanWidth
	add pixPtr , 1 
	b predict


  # load the pixel in question 


loadPix:
	add pixPtr , 1
predict:
	sub diff , pixel , pred # calculate differrence value
	move pred , pixel # use pixel as next predictor
	and diff , 0x1ff # clamp to nine bit
	sll diff , 3 # and shift 3 bits to left to use as table index

	# load huffman code / size pair into regs
	add diff , huffP , diff
	ld huffCode , (diff)
	# reduce amount of free bits by actual size of coded symbol
	sub huffBitsFree, huffBitsFree , huffSize
	# shift code to left to fit the place
	sll huffCode , huffCode , huffBitsFree
	# and or this  ...
	or huffBits , huffBits , huffCode

	# ready to emit coded data
emitHuff:
	bgt huffBitsFree , 16 , end_pixel

	# get 8 higher bits from 
	srl work , huffBits , 24 # always need this ..
	sb work , (outP) # store one byte
	bne work , 0xff , after_stuff

	add outP , 1 # increment destination pointer
	sb zero , (outP) # and stuffed 0 too
after_stuff:
	srl work , huffBits , 16
	and work , 0xff
	sb work , 1(outP)
	bne work , 0xff , after_stuff2
	sb zero , 2(outP)
	add outP , 1

after_stuff2:
	add outP , 2
	sll huffBits , huffBits , 16
	add huffBitsFree , 16
	# we are ready with pixel ...
end_pixel:
	sub pix2do , 1
	lbu pixel , (pixPtr)
	bnez pix2do , loadPix

	sub row2do , 1
	move pred , predFirst
	bnez row2do , restartRow
	# init first predictor for the pixel


flush:
	# flush huffman encoder
	bgt huffBitsFree , 24 , flush_last
	srl work , huffBits , 24
	sll huffBits , huffBits , 8
	sb work , (outP)
	bne work , 0xff , after_flush_stuff # even here we must check for 0xff

	add outP , 1
	sb zero , (outP)

after_flush_stuff:
	add outP , 1
flush_last:
	li huffCode , 0x1ff
	sub huffBitsFree , huffBitsFree , 9
	sll huffCode , huffCode , huffBitsFree
	# encode this
	or huffBits , huffBits , huffCode
	srl work , huffBits , 24
	sb work , (outP)
	bne work , 0xff , after_flush_last_stuff

	add outP , 1
	sb zero , (outP)

after_flush_last_stuff:


  # write EOI marker
	li work , 0xff
	sb work , 1(outP)
	li work , 0xd9
	sb work , 2(outP)
	add outP , 3
  # compose output result
	sub $2 , outP , outPsave
	

	.livereg	0x2000FF0E,0x00000FFF
	j	$31
	.end	encodeScan
