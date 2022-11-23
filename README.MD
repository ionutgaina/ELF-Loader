# ELF-Loader

Implemented a very simplified version of ELF Loader

Implementation:
- Looked for the problem segment.
- If the segment doesn't exist, 
ran with the default handler for page fault.
- Calculated the start of the page address from the problem segment.
- Calculated the offset of the page from 
the beginning of the address of the problem segment.
- If signal code of the handler is 1 (SEGV_MAPPER), then
we mapp the page, and read from the file to mapped map, after that
we give the needed permissions for mapped page.
- If signal code isn't 1, 
then we ran with the default handler for page fault.