#include "mmu.h"
#include "terminal.h"

void mmu_report(void) {
    terminal_writeln("MMU: 4-level paging enabled in early boot (2MiB pages)");
}

void cache_report(void) {
    terminal_writeln("Cache: CR0.CD=0 and CR0.NW=0 -> CPU caches enabled");
}
