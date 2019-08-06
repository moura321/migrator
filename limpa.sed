s/^.*: system.mem_ctrls: access wrote /M/
s/^.*: global: IFetch from .cpu.inst of size /I/
s/^.*: global: Write from .cpu.data of size /W/
s/^.*: global: Read from .cpu.data of size /R/
/^[^MIWR]/d
s/on address 0x//
s/ bytes to address//
s/ data .*//

