#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<elf.h>
#include<fcntl.h>

int main(int argc,char *argv[])
{
	int fd;
	int i, symcount,j;
	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr,*symshdr,*shdrp, *sh;
	Elf64_Sym  *sym;
	struct stat buf;
	unsigned char *mem;
	char *symbols, *symbol;
	char name[256],name1[256];

	if(argc != 2)
	{
		perror("Please pass a name of an executable file..\n");
		exit(1);
	}

	if((fd = open(argv[1],O_RDWR)) < 0)
	{
		perror("Could not open the file provided\n");
		exit(1);
	}

	if(fstat(fd,&buf) < 0)
	{
		perror("fstat failed...Error\n");
		exit(1);
	}

	mem = mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);

	if(mem == MAP_FAILED)
	{
		perror("mmap failed...\n");
		exit(1);
	}

	ehdr = (Elf64_Ehdr *)mem;
	shdr = (Elf64_Shdr *)(mem + ehdr->e_shoff);

	shdrp = shdr;

	/*This code below is used to be able to get the "section header string table section" as it is obtained from the "e_shstrndx" of the ELF header */
	sh = &shdr[ehdr->e_shstrndx];
	symbol = malloc(sh->sh_size);
	memcpy(symbol,(mem + sh->sh_offset),sh->sh_size);

	for(i= ehdr->e_shnum; i-- > 0;shdrp++)
	{
		printf("Section Name: %s\n",&symbol[shdrp->sh_name]);
		if(shdrp->sh_type == SHT_DYNSYM)
		{
			printf("############----DYNAMIC SYMBOLS-----#############\n\n");
			symshdr = &shdr[shdrp->sh_link];
			printf("Size of the symbol header table section %d\n",symshdr->sh_size);
			if((symbols = malloc(symshdr->sh_size)) == NULL)
			{
				perror("malloc failed to create symbols memory..\n");
				exit(1);
			}

			if(memcpy(symbols,(mem + symshdr->sh_offset), symshdr->sh_size) == NULL)
			{
				perror("memcpy failed..\n");
				exit(1);
			}

			sym = (Elf64_Sym *)malloc(shdrp->sh_size);

			if(memcpy((Elf64_Sym *)sym , (Elf64_Sym *)(mem + shdrp->sh_offset), shdrp->sh_size) == NULL)
			{
				perror("memcpy failed 2..\n");
				exit(1);
			}

			symcount = shdrp->sh_size / sizeof(Elf64_Sym);
			printf("Symbols count: %d\n",symcount);
			printf("Entries:  %d\n",shdrp->sh_entsize);

			for(j=0;j< symcount; j++,sym++)
			{
				strncpy(name,&symbols[sym->st_name],sizeof(name) -1);
				printf("Name of symbol %s\n",name);
			}
			printf("\n##############################\n");

		}
	}
	return 0;
}

