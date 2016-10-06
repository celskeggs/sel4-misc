#ifndef LIB_ELFLOADER_ELFCONTEXT_H
#define LIB_ELFLOADER_ELFCONTEXT_H

// TCB is not provided to subprocess.
#include <resource/untyped.h>

// TODO: get 4 (log 16, the cnode entry size) as a constant
#define ECAP_ROOT_BITS (BITS_4KIB - 4)

enum elfcontext_cspace {
    ecap_Null = 0,
    ecap_PD,
    ecap_CNode,
    ecap_IPC,
    ecap_IOEP,
    ecap_StartFree,
    ecap_EndFree = BIT(ECAP_ROOT_BITS) - 1,
};

// messages to root from sandbox
enum root_label {
    RL_TEST = 256,
    RL_HALT,
};

// brief architecture overview:

// ---- privileged servers ----
// root server: controls servers and manages memory. communicates only with sandbox server.
// sandbox server: controls i/o overview between processes. all processes start only with an endpoint for this server.
//                 relays connection requests between servers and relays memory and process creation requests to root server.
// policy server: tracks privileges assigned to each server; used by sandbox to determine which operations are allowed.
// ---- system servers ----
// monitor server(s): contain device drivers to talk to user via serial, text-mode VGA, graphics, etc. contain multiplexing code to let user control monitor inputs to servers.
// bus server(s): contain device drivers for interconnects (shared interrupt lines, PCI, USB).
// disk server(s): contain device drivers to talk to disks.
// filesystem server(s): contain filesystem handling code.
// storage server: interconnect between filesystem servers and individual applications.
// network card server(s): contain device drivers to talk to network cards.
// network server: IP routing, firewalls, and isolation.
// sound card server(s): contain device drivers to talk to sound cards.
// sound server: sound mixing and routing.
// power server: contains device drivers for ACPI, etc. and manages system power state.
// installation server: capable of installing programs. checks signatures, etc.
// ---- core applications ----
// storage management app: file transfer handling.
// process app: monitor and control running servers.
// policy app: configure and reconfigure policies.
// powerbox app: transiently permit specific privileges.
// power app: interface for controlling shutdown, sleep, hibernate, etc. also laptop batteries, charging, etc.
// launcher app: launch other applications
// installation app: has permission to install programs.



// note that individual servers are very much allowed to create their own sub-TCBs! only top-level process creation is handled like this.


#endif //LIB_ELFLOADER_ELFCONTEXT_H
