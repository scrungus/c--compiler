.globl premain
.text
premain:
# Creating new frame
addiu $sp, $sp -12
li $t1, 12
sw $t1, 0($sp)
li $t2, 0
sw $t2, 8($sp)
li $t3, 0
sw $t3, 4($sp)
# End of creating frame
#saving global frame
# Saving frame
# End of saving frame
jal main
main:
# Creating new frame
addiu $sp, $sp -12
li $t1, 12
sw $t1, 0($sp)
li $t2, 0
sw $t2, 8($sp)
li $t3, 1
sw $t3, 4($sp)
# End of creating frame

# Saving frame
# End of saving frame

li $a0,10
jal fact
# Restoring frame
# End of saving frame

fact:
# Creating new frame
addiu $sp, $sp -24
li $t1, 24
sw $t1, 0($sp)
li $t2, 3
sw $t2, 8($sp)
li $t3, 2
sw $t3, 4($sp)
# End of creating frame

move $t0 $a0
# Saving frame

sw $t0 12($sp)
# End of saving frame

li $a0,1
move $a1,$t0
jal inner_fact
# Restoring frame

lw $t0  16($sp)
# End of saving frame
jr $ra
inner_fact:
# Creating new frame
addiu $sp, $sp -28
li $t1, 28
sw $t1, 0($sp)
li $t2, 4
sw $t2, 8($sp)
li $t3, 3
sw $t3, 4($sp)
# End of creating frame

move $t2 $a0
move $t3 $a1
li $t4,0
bne $t3 $t4 L1
lw $v1 t2
addiu $sp, $sp 28
jr $ra
L1:
move $t0,$t2
move $t1,$t3
mul $t2,$t0,$t1

move $t1,$t3
li $t0,1
sub $t3,$t1,$t0

# Saving frame
sw $t3 12($sp)
sw $t2 16($sp)
# End of saving frame

move $a0,$t2
move $a1,$t3
jal inner_fact
# Restoring frame
lw $t3  12($sp)
lw $t2  16($sp)
# End of saving frame
jr $ra

move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
