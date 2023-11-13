# file: isr_timer.s

.section isr
# prekidna rutina za tajmer
.global isr_timer
.extern nepoznati
isr_timer:
my_start:
    ret

.end
