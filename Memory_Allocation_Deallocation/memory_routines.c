

/**************************************************************************

The commented out printf and main() can be used for testing / debugging

**************************************************************************/

#include"memory_routines.h"

void *alloc_mem(unsigned amount){
	printf("Begin alloc_mem\n");
	unsigned int finalAmount = amount;
	if(amount % 16 > 0){
		finalAmount = (16 * ((amount / 16) + 1)) + (2*sizeof(struct tag_block));
	}
	else{
		finalAmount = amount + (2*sizeof(struct tag_block));
	}

	struct tag_block *tagBlock = NULL;
	struct free_block *freeBlock = NULL;
	void *ptr = NULL;
	struct free_block *new = header->back_link;
	char *tmpPtr = (char*)new;


	while(ptr == NULL){
		/*printf("*Header: 0x%X\n", (unsigned int)header);
		printf("*New: 0x%X\n", (unsigned int)new);*/
		if(new == header){
			printf("-------------------------------------\n");
			printf("Not enough free memory\n");
			printf("Returning NULL\n");
			printf("-------------------------------------\n");
			return NULL;
		}
		tmpPtr -= sizeof(struct tag_block);
		tagBlock = (struct tag_block*)(tmpPtr);
		/*printf("sizeof(tagblock): %d\n", (int)sizeof(struct tag_block));
		printf("tagblock: 0x%X\n", (unsigned)tagBlock);
		printf("tagblock->tag: %c\n", tagBlock->tag);
		printf("tagblock->sig: %s\n", tagBlock->sig);
		printf("tagblock->size: %d\n", tagBlock->size);
		printf("finalAmount: %d\n", finalAmount);*/
		if(tagBlock->size >= (finalAmount - (2*sizeof(struct tag_block)))){
			unsigned int freesize = tagBlock->size;
			unsigned int reqRemainSize = ((2*sizeof(struct tag_block))+sizeof(struct free_block));
			unsigned int useAllocSize = finalAmount - (2*sizeof(struct tag_block));
			unsigned int allocSize = finalAmount - sizeof(struct tag_block);
			unsigned int remainSize = freesize - allocSize + sizeof(struct tag_block);
			unsigned int remainAllocSize = freesize - allocSize - sizeof(struct tag_block);
			/* allocate part of the free space and re-bound the rest */
			if(remainSize >= reqRemainSize){
				tagBlock = (struct tag_block*)tmpPtr;
				tagBlock->size = remainAllocSize;

				tmpPtr += sizeof(struct tag_block) + remainAllocSize;
				tagBlock = (struct tag_block*)tmpPtr;
				tagBlock->tag = '0';
				strcpy(tagBlock->sig, "end_memblk");
				tagBlock->size = remainAllocSize;

				tmpPtr += sizeof(struct tag_block);
				tagBlock = (struct tag_block*)tmpPtr;
				tagBlock->tag = '1';
				strcpy(tagBlock->sig, "top_memblk");
				tagBlock->size = useAllocSize;

				ptr = (void*)(tmpPtr + sizeof(struct tag_block));
				tmpPtr += (sizeof(struct tag_block) + useAllocSize);
				tagBlock = (struct tag_block*)tmpPtr;
				tagBlock->tag = '1';
				strcpy(tagBlock->sig, "end_memblk");
				tagBlock->size = useAllocSize;
			}
			/* allocate entire free space */
			else{
				tagBlock = (struct tag_block*)tmpPtr;
				tagBlock->tag = '1';

				tmpPtr += sizeof(struct tag_block);
				ptr = (void*)tmpPtr;
				freeBlock = (struct free_block*) tmpPtr;
				freeBlock->fwd_link->back_link = freeBlock->back_link;
				freeBlock->back_link->fwd_link = freeBlock->fwd_link;

				tmpPtr += freesize;
				tagBlock = (struct tag_block*)tmpPtr;
				tagBlock->tag = '1';
			}
			return ptr;
		}
		else{
			new = new->back_link;
		}

	}
	printf("End alloc_mem\n");
	return ptr;
}

unsigned int release_mem( void *ptr ){
	printf("Begin release_mem\n");
	char *tmpPtr = (char*) ptr;

	/* check validity */
	tmpPtr -= sizeof(struct tag_block);
	struct tag_block *tagBlockBegin = (struct tag_block*)tmpPtr;

	if(tagBlockBegin->tag != '1' || strcmp(tagBlockBegin->sig, "top_memblk") != 0){
		printf("Detected an invalid tag preceding given ptr\n");
		return 1;
	}

	tmpPtr -= sizeof(struct tag_block);
	struct tag_block *tagBlockAbove = (struct tag_block*)tmpPtr;
	tmpPtr -= (tagBlockAbove->size + sizeof(struct tag_block));
	struct tag_block *tagBlockTop = (struct tag_block*)tmpPtr;
	tmpPtr += ((tagBlockTop->size + tagBlockBegin->size) + (3*sizeof(struct tag_block)));
	struct tag_block *tagBlockEnd = (struct tag_block*)tmpPtr;
	tmpPtr += sizeof(struct tag_block);
	struct tag_block *tagBlockBelow = (struct tag_block*)tmpPtr;
	tmpPtr += tagBlockBelow->size + sizeof(struct tag_block);
	struct tag_block *tagBlockBottom = (struct tag_block*)tmpPtr;

	/*printf("\n\n\n##########################\n");
	printf("tagblockTop: 0x%X\n", tagBlockTop);
	printf("tagblock->tag: %c\n", tagBlockTop->tag);
	printf("tagblock->sig: %s\n", tagBlockTop->sig);
	printf("tagblock->size: %d\n", tagBlockTop->size);
	printf("tagblockAbove: 0x%X\n", tagBlockAbove);
	printf("tagblock->tag: %c\n", tagBlockAbove->tag);
	printf("tagblock->sig: %s\n", tagBlockAbove->sig);
	printf("tagblock->size: %d\n", tagBlockAbove->size);
	printf("tagblockBegin: 0x%X\n", tagBlockBegin);
	printf("tagblock->tag: %c\n", tagBlockBegin->tag);
	printf("tagblock->sig: %s\n", tagBlockBegin->sig);
	printf("tagblock->size: %d\n", tagBlockBegin->size);
	printf("tagblockEnd: 0x%X\n", tagBlockEnd);
	printf("tagblock->tag: %c\n", tagBlockEnd->tag);
	printf("tagblock->sig: %s\n", tagBlockEnd->sig);
	printf("tagblock->size: %d\n", tagBlockEnd->size);
	printf("tagblockBelow: 0x%X\n", tagBlockBelow);
	printf("tagblock->tag: %c\n", tagBlockBelow->tag);
	printf("tagblock->sig: %s\n", tagBlockBelow->sig);
	printf("tagblock->size: %d\n", tagBlockBelow->size);
	printf("tagblockBottom: 0x%X\n", tagBlockBottom);
	printf("tagblock->tag: %c\n", tagBlockBottom->tag);
	printf("tagblock->sig: %s\n", tagBlockBottom->sig);
	printf("tagblock->size: %d\n########################\n", tagBlockBottom->size);*/

	struct free_block *freeBlock = NULL;
	printf("1\n");
	if(tagBlockAbove->tag == '1' && tagBlockBelow->tag == '1'){
		tmpPtr = (char*) tagBlockBegin;
		strcpy(tagBlockBegin->sig, "top_memblk");
		tagBlockBegin->tag = '0';

		tmpPtr += sizeof(struct tag_block);
		freeBlock = (struct free_block*)tmpPtr;
		header->back_link->fwd_link = freeBlock;
		freeBlock->back_link = header->back_link;
		header->back_link = freeBlock;
		freeBlock->fwd_link = header;

		tagBlockEnd->tag = '0';
		strcpy(tagBlockEnd->sig, "end_memblk");

		/* clears out previously allocated memory */
		tmpPtr = (char*) tagBlockBegin;
		tmpPtr+= (sizeof(struct tag_block) + sizeof(struct free_block));
		unsigned int blankSize = tagBlockBegin->size - sizeof(struct free_block);
		char* blankmem = calloc(1, blankSize);
		memcpy(tmpPtr, blankmem, blankSize);
		free(blankmem);

		/*printf("\n\n\n***********************\ncase 1,1\n");
		printf("tagblockTop: 0x%X\n", tagBlockTop);
		printf("tagblock->tag: %c\n", tagBlockTop->tag);
		printf("tagblock->sig: %s\n", tagBlockTop->sig);
		printf("tagblock->size: %d\n", tagBlockTop->size);
		printf("tagblockAbove: 0x%X\n", tagBlockAbove);
		printf("tagblock->tag: %c\n", tagBlockAbove->tag);
		printf("tagblock->sig: %s\n", tagBlockAbove->sig);
		printf("tagblock->size: %d\n", tagBlockAbove->size);
		printf("tagblockBegin: 0x%X\n", tagBlockBegin);
		printf("tagblock->tag: %c\n", tagBlockBegin->tag);
		printf("tagblock->sig: %s\n", tagBlockBegin->sig);
		printf("tagblock->size: %d\n", tagBlockBegin->size);
		printf("tagblockEnd: 0x%X\n", tagBlockEnd);
		printf("tagblock->tag: %c\n", tagBlockEnd->tag);
		printf("tagblock->sig: %s\n", tagBlockEnd->sig);
		printf("tagblock->size: %d\n", tagBlockEnd->size);
		printf("tagblockBelow: 0x%X\n", tagBlockBelow);
		printf("tagblock->tag: %c\n", tagBlockBelow->tag);
		printf("tagblock->sig: %s\n", tagBlockBelow->sig);
		printf("tagblock->size: %d\n", tagBlockBelow->size);
		printf("tagblockBottom: 0x%X\n", tagBlockBottom);
		printf("tagblock->tag: %c\n", tagBlockBottom->tag);
		printf("tagblock->sig: %s\n", tagBlockBottom->sig);
		printf("tagblock->size: %d\n****************\n", tagBlockBottom->size);*/
	}
	else if(tagBlockAbove->tag == '0' && tagBlockBelow->tag == '1'){
		unsigned int totalSize = tagBlockTop->size + (2*sizeof(struct tag_block)) + tagBlockBegin->size;
		tagBlockTop->tag = '0';
		strcpy(tagBlockTop->sig, "top_memblk");
		tagBlockTop->size = totalSize;

		tagBlockEnd->tag = '0';
		strcpy(tagBlockEnd->sig, "end_memblk");
		tagBlockEnd->size = totalSize;

		strcpy(tagBlockAbove->sig, "old_end_mb");
		strcpy(tagBlockBegin->sig, "old_top_mb");

		tmpPtr = (char*) tagBlockTop;
		tmpPtr+= (sizeof(struct tag_block) + sizeof(struct free_block));
		unsigned int blankSize = totalSize - sizeof(struct free_block);
		char* blankmem = calloc(1, blankSize);
		memcpy(tmpPtr, blankmem, blankSize);
		free(blankmem);

		/*printf("\n\n\n***********************\ncase 0,1\n");
		printf("tagblockTop: 0x%X\n", tagBlockTop);
		printf("tagblock->tag: %c\n", tagBlockTop->tag);
		printf("tagblock->sig: %s\n", tagBlockTop->sig);
		printf("tagblock->size: %d\n", tagBlockTop->size);
		printf("tagblockAbove: 0x%X\n", tagBlockAbove);
		printf("tagblock->tag: %c\n", tagBlockAbove->tag);
		printf("tagblock->sig: %s\n", tagBlockAbove->sig);
		printf("tagblock->size: %d\n", tagBlockAbove->size);
		printf("tagblockBegin: 0x%X\n", tagBlockBegin);
		printf("tagblock->tag: %c\n", tagBlockBegin->tag);
		printf("tagblock->sig: %s\n", tagBlockBegin->sig);
		printf("tagblock->size: %d\n", tagBlockBegin->size);
		printf("tagblockEnd: 0x%X\n", tagBlockEnd);
		printf("tagblock->tag: %c\n", tagBlockEnd->tag);
		printf("tagblock->sig: %s\n", tagBlockEnd->sig);
		printf("tagblock->size: %d\n", tagBlockEnd->size);
		printf("tagblockBelow: 0x%X\n", tagBlockBelow);
		printf("tagblock->tag: %c\n", tagBlockBelow->tag);
		printf("tagblock->sig: %s\n", tagBlockBelow->sig);
		printf("tagblock->size: %d\n", tagBlockBelow->size);
		printf("tagblockBottom: 0x%X\n", tagBlockBottom);
		printf("tagblock->tag: %c\n", tagBlockBottom->tag);
		printf("tagblock->sig: %s\n", tagBlockBottom->sig);
		printf("tagblock->size: %d\n****************\n", tagBlockBottom->size);*/
	}
	else if(tagBlockAbove->tag == '1' && tagBlockBelow->tag == '0'){
		unsigned int totalSize = tagBlockBegin->size + (2*sizeof(struct tag_block)) + tagBlockBottom->size;
		tagBlockBegin->tag = '0';
		strcpy(tagBlockBegin->sig, "top_memblk");
		tagBlockBegin->size = totalSize;

		tagBlockBottom->tag = '0';
		strcpy(tagBlockBottom->sig, "eng_memblk");
		tagBlockBottom->size = totalSize;

		strcpy(tagBlockEnd->sig, "old_end_mb");
		strcpy(tagBlockBelow->sig, "old_top_mb");

		tmpPtr = (char*) tagBlockBegin;
		tmpPtr += sizeof(struct tag_block);
		freeBlock = (struct free_block*)tmpPtr;
		tmpPtr = (char*) tagBlockBelow;
		tmpPtr += sizeof(struct tag_block);
		struct free_block *tmpFB = (struct free_block*)tmpPtr;

		tmpFB->back_link->fwd_link = freeBlock;
		tmpFB->fwd_link->back_link = freeBlock;
		freeBlock->back_link = tmpFB->back_link;
		freeBlock->fwd_link = tmpFB->fwd_link;

		tmpPtr = (char*) tagBlockBegin;
		tmpPtr+= (sizeof(struct tag_block) + sizeof(struct free_block));
		unsigned int blankSize = totalSize - sizeof(struct free_block);
		char* blankmem = calloc(1, blankSize);
		memcpy(tmpPtr, blankmem, blankSize);
		free(blankmem);

		/*printf("\n\n\n***********************\ncase 1,0\n");
		printf("tagblockTop: 0x%X\n", tagBlockTop);
		printf("tagblock->tag: %c\n", tagBlockTop->tag);
		printf("tagblock->sig: %s\n", tagBlockTop->sig);
		printf("tagblock->size: %d\n", tagBlockTop->size);
		printf("tagblockAbove: 0x%X\n", tagBlockAbove);
		printf("tagblock->tag: %c\n", tagBlockAbove->tag);
		printf("tagblock->sig: %s\n", tagBlockAbove->sig);
		printf("tagblock->size: %d\n", tagBlockAbove->size);
		printf("tagblockBegin: 0x%X\n", tagBlockBegin);
		printf("tagblock->tag: %c\n", tagBlockBegin->tag);
		printf("tagblock->sig: %s\n", tagBlockBegin->sig);
		printf("tagblock->size: %d\n", tagBlockBegin->size);
		printf("tagblockEnd: 0x%X\n", tagBlockEnd);
		printf("tagblock->tag: %c\n", tagBlockEnd->tag);
		printf("tagblock->sig: %s\n", tagBlockEnd->sig);
		printf("tagblock->size: %d\n", tagBlockEnd->size);
		printf("tagblockBelow: 0x%X\n", tagBlockBelow);
		printf("tagblock->tag: %c\n", tagBlockBelow->tag);
		printf("tagblock->sig: %s\n", tagBlockBelow->sig);
		printf("tagblock->size: %d\n", tagBlockBelow->size);
		printf("tagblockBottom: 0x%X\n", tagBlockBottom);
		printf("tagblock->tag: %c\n", tagBlockBottom->tag);
		printf("tagblock->sig: %s\n", tagBlockBottom->sig);
		printf("tagblock->size: %d\n****************\n", tagBlockBottom->size);*/
	}
	else if(tagBlockAbove->tag == '0' && tagBlockBelow->tag == '0'){
		unsigned int totalSize = tagBlockTop->size + tagBlockBegin->size + tagBlockBelow->size + (4*sizeof(struct tag_block));
		tagBlockTop->tag = '0';
		strcpy(tagBlockTop->sig, "top_memblk");
		tagBlockTop->size = totalSize;

		tagBlockBottom->tag = '0';
		strcpy(tagBlockBottom->sig, "end_memblk");
		tagBlockBottom->size = totalSize;

		strcpy(tagBlockAbove->sig, "old_end_mb");
		strcpy(tagBlockBegin->sig, "old_top_mb");
		strcpy(tagBlockEnd->sig, "old_end_mb");
		strcpy(tagBlockBelow->sig, "old_top_mb");

		tmpPtr = (char*)tagBlockBelow;
		tmpPtr += sizeof(struct tag_block);
		freeBlock = (struct free_block*)tmpPtr;
		freeBlock->fwd_link->back_link = freeBlock->back_link;
		freeBlock->back_link->fwd_link = freeBlock->fwd_link;

		tmpPtr = (char*) tagBlockTop;
		tmpPtr+= (sizeof(struct tag_block) + sizeof(struct free_block));
		unsigned int blankSize = totalSize - sizeof(struct free_block);
		char* blankmem = calloc(1, blankSize);
		memcpy(tmpPtr, blankmem, blankSize);
		free(blankmem);

		/*printf("\n\n\n***********************\ncase 0,0\n");
		printf("tagblockTop: 0x%X\n", tagBlockTop);
		printf("tagblock->tag: %c\n", tagBlockTop->tag);
		printf("tagblock->sig: %s\n", tagBlockTop->sig);
		printf("tagblock->size: %d\n", tagBlockTop->size);
		printf("tagblockAbove: 0x%X\n", tagBlockAbove);
		printf("tagblock->tag: %c\n", tagBlockAbove->tag);
		printf("tagblock->sig: %s\n", tagBlockAbove->sig);
		printf("tagblock->size: %d\n", tagBlockAbove->size);
		printf("tagblockBegin: 0x%X\n", tagBlockBegin);
		printf("tagblock->tag: %c\n", tagBlockBegin->tag);
		printf("tagblock->sig: %s\n", tagBlockBegin->sig);
		printf("tagblock->size: %d\n", tagBlockBegin->size);
		printf("tagblockEnd: 0x%X\n", tagBlockEnd);
		printf("tagblock->tag: %c\n", tagBlockEnd->tag);
		printf("tagblock->sig: %s\n", tagBlockEnd->sig);
		printf("tagblock->size: %d\n", tagBlockEnd->size);
		printf("tagblockBelow: 0x%X\n", tagBlockBelow);
		printf("tagblock->tag: %c\n", tagBlockBelow->tag);
		printf("tagblock->sig: %s\n", tagBlockBelow->sig);
		printf("tagblock->size: %d\n", tagBlockBelow->size);
		printf("tagblockBottom: 0x%X\n", tagBlockBottom);
		printf("tagblock->tag: %c\n", tagBlockBottom->tag);
		printf("tagblock->sig: %s\n", tagBlockBottom->sig);
		printf("tagblock->size: %d\n****************\n", tagBlockBottom->size);*/
	}
	printf("End release_mem\n");
	return 0;
}


int main(){
	unsigned int allsize = 640;

	unsigned int size = sizeof(struct tag_block) + sizeof(struct tag_block)
			+ sizeof(struct free_block) + allsize
			+ sizeof(struct tag_block) + sizeof(struct tag_block)
			+ sizeof(struct free_block);

	char *allocmem = (char*) calloc(1, size);
	if(allocmem == NULL){
		printf("calloc failed\n");
		exit(0);
	}
	printf("Memory allocated at: 0x%X\n", (unsigned int) allocmem);
	char* tmpPtr = allocmem;
	header = (struct free_block*) (allocmem + (size - sizeof(struct free_block)));

	struct tag_block* tagBlock = NULL;
	struct free_block* freeBlock = NULL;

	struct tag_block* topRegion = (struct tag_block*)tmpPtr;
	topRegion->tag = '1';
	strcpy(topRegion->sig, "end_region");
	topRegion->size = 0;

	tmpPtr += sizeof(struct tag_block);
	struct tag_block* regionBegin = (struct tag_block*)tmpPtr;
	regionBegin->tag = '0';
	strcpy(regionBegin->sig, "top_memblk");
	regionBegin->size = allsize;

	tmpPtr += sizeof(struct tag_block);
	struct free_block* freePtr = (struct free_block*) tmpPtr;

	tmpPtr += regionBegin->size;
	struct tag_block* regionEnd = (struct tag_block*)tmpPtr;
	regionEnd->tag = '0';
	strcpy(regionEnd->sig, "end_memblk");
	regionEnd->size = allsize;

	tmpPtr += sizeof(struct tag_block);
	struct tag_block* bottomRegion = (struct tag_block*) tmpPtr;
	bottomRegion->tag = '1';
	strcpy(bottomRegion->sig, "top_region");
	bottomRegion->size = 0;

	tmpPtr += sizeof(struct tag_block);
	header = (struct free_block*)tmpPtr;
	header->back_link = freePtr;
	header->fwd_link = freePtr;
	freePtr->back_link = header;
	freePtr->fwd_link = header;

	char num = 'y';
	int* mem = (int*) calloc(1, sizeof(void*));
	int sze = 0;
	while(tolower(num) != 'n'){
		int number;
		printf("Enter operation\n1 - alloc_mem()\n2 - release_mem()\n:");
		scanf("%d", &number);
		if(number == 1){
			if((mem = realloc(mem,sze+1)) == NULL){
				printf("error during realloc\n");
			}
			int arg;
			printf("Enter amount of memory: ");
			scanf("%d", &arg);
			if((mem[sze] = alloc_mem(arg)) == NULL){
				printf("alloc_mem returned NULL\n");
			}
			else{
				sze++;
			}
		}
		else if(number == 2){
			int userin;
			int i;
			printf("Choose a pointer:\n");
			for(i = 0; i< sze; i++){
				printf("%d: 0x%X\n", i, (unsigned int) mem[i]);
			}
			scanf("%d", &userin);
			if(release_mem(mem[userin]) != 0){
				printf("release_mem returned non-zero\n");
			}
			else{
			}
		}
		printf("Continue (y/n): ");
		scanf("%c", &num);
	}

	free(allocmem);
}


