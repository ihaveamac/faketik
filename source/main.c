#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "basetik_bin.h"

struct ticket_dumb {
	u8 unused1[0x1DC];
	u64 titleID_be;
	u8 unused2[0x16C];
} __attribute__((packed));

void fix_unused_tickets(void) {
	Handle ticketHandle;
	Result res;
	struct ticket_dumb *buf;
	u32 i, j;

	u32 titlesNANDCount, titlesSDMCCount, ticketsCount;
	u32 titlesNANDRead, titlesSDMCRead, ticketsRead;
	u32 titleTotalCount, titleTotalRead;
	u32 missingCount = 0;

	u64 *titles;
	u64 *tickets;
	u64 *missingTickets;

	bool etd_available;
	res = AM_QueryAvailableExternalTitleDatabase(&etd_available);
	if (R_FAILED(res)) {
		printf("Failed to query the exteral title database.\n  AM_QueryAvailableExternalTitleDatabase:\n  0x%08lx", res);
		return;
	}

	if (!etd_available) {
		puts("The External Title Database is not available.");
		return;
	}

	// get titles
	res = AM_GetTitleCount(MEDIATYPE_NAND, &titlesNANDCount);
	res = AM_GetTitleCount(MEDIATYPE_SD, &titlesSDMCCount);

	titleTotalCount = titlesNANDCount + titlesSDMCCount;

	titles = calloc(titleTotalCount, sizeof(u64));
	if (!titles) {
		puts("Failed to allocate memory for titles.\nThis should not happen.");
		return;
	}

	res = AM_GetTitleList(&titlesNANDRead, MEDIATYPE_NAND, titlesNANDCount, titles);
	res = AM_GetTitleList(&titlesSDMCRead, MEDIATYPE_SD, titlesSDMCRead, titles + titlesNANDRead);

	titleTotalRead = titlesNANDRead + titlesSDMCRead;

	// get tickets
	res = AM_GetTicketCount(&ticketsCount);

	tickets = calloc(ticketsCount, sizeof(u64));
	if (!tickets) {
		puts("Failed to allocate memory for tickets.\nThis should not happen.");
		free(titles);
		return;
	}

	missingTickets = calloc(titleTotalRead, sizeof(u64));
	if (!missingTickets) {
		puts("Failed to allocate memory for missing tickets.\nThis should not happen.");
		free(titles);
		free(tickets);
		return;
	}

	res = AM_GetTicketList(&ticketsRead, ticketsCount, 0, tickets);

	if (R_FAILED(res)) {
		printf("Getting ticket or title information.\nI don't think this should happen...\n  err: 0x%08lx\n", res);
		free(titles);
		free(tickets);
		free(missingTickets);
		return;
	}

	// get missing tickets
	u64 currentTitle;
	bool found;
	for (i = 0; i < titleTotalRead; i++) {
		currentTitle = titles[i];
		found = false;
		for (j = 0; j < ticketsRead; j++) {
			if (currentTitle == tickets[j]) {
				found = true;
				break;
			}
		}

		if (!found) {
			fputs("", stdout); // this fixes an issue and I have no fucking clue why.
			missingTickets[missingCount] = currentTitle;
			missingCount++;
		}
	}

	free(titles);
	free(tickets);

	if (!missingCount) {
		puts("No missing tickets.");
		free(missingTickets);
		return;
	}

	// now we install the tickets...
	buf = malloc(basetik_bin_size);
	if (!buf) {
		puts("Failed to allocate memory for the ticket buf.\nThis should not happen.");
		free(missingTickets);
		return;
	}

	memcpy(buf, basetik_bin, basetik_bin_size);

	for (i = 0; i < missingCount; i++) {
		buf->titleID_be = __builtin_bswap64(missingTickets[i]);
		res = AM_InstallTicketBegin(&ticketHandle);
		if (R_FAILED(res)) {
			printf("Ticket install for %016llx failed:\n  AM_InstallTicketBegin: 0x%08lx\n", missingTickets[i], res);
			AM_InstallTicketAbort(ticketHandle);
			continue;
		}

		res = FSFILE_Write(ticketHandle, NULL, 0, buf, sizeof(struct ticket_dumb), 0);
		if (R_FAILED(res)) {
			printf("Ticket install for %016llx failed:\n  FSFILE_Write: 0x%08lx\n", missingTickets[i], res);
			AM_InstallTicketAbort(ticketHandle);
			continue;
		}

		res = AM_InstallTicketFinish(ticketHandle);
		if (R_FAILED(res)) {
			printf("Ticket install for %016llx failed:\n  AM_InstallTicketFinish: 0x%08lx\n", missingTickets[i], res);
			AM_InstallTicketAbort(ticketHandle);
			continue;
		}

		printf("Installed ticket for %016llx.\n", missingTickets[i]);

	}

	free(buf);
	free(missingTickets);
	puts("Finished fixing titles.");
}

int main(int argc, char* argv[])
{
	amInit();
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	puts("faketik v1.1");

	puts("Starting to fix tickets...");
	fix_unused_tickets();
	puts("\nPress START or B to exit.");

	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START || kDown & KEY_B)
			break;
	}

	gfxExit();
	amExit();
	return 0;
}
