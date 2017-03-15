#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include "api.h"
#include "cJSON.h"

Result http_get(const char *url, char ** response_data) {
	Result ret=0;
	httpcContext context;
	char *newurl=NULL;
	u32 statuscode=0;
	u32 contentsize=0, readsize=0, size=0;
	u8 *buf, *lastbuf;

	do {
		ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
		// This disables SSL cert verification, so https:// will be usable
		ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);

		// Set a User-Agent header so websites can identify your application
		ret = httpcAddRequestHeaderField(&context, "User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36");

		// Tell the server we can support Keep-Alive connections.
		// This will delay connection teardown momentarily (typically 5s)
		// in case there is another request made to the same server.
		ret = httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");

		ret = httpcBeginRequest(&context);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		ret = httpcGetResponseStatusCode(&context, &statuscode);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
			if(newurl==NULL) newurl = malloc(0x1000); // One 4K page for new URL
			if (newurl==NULL){
				httpcCloseContext(&context);
				return -1;
			}
			ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
			url = newurl; // Change pointer to the url that we just learned
			httpcCloseContext(&context); // Close this context before we try the next
		}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));

	if(statuscode!=200){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -2;
	}

	// This relies on an optional Content-Length header and may be 0
	ret=httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return ret;
	}

	// Start with a single page buffer
	buf = (u8*)malloc(0x1000);
	if(buf==NULL){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -1;
	}

	do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize;
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
				lastbuf = buf; // Save the old pointer, in case realloc() fails.
				buf = realloc(buf, size + 0x1000);
				if(buf==NULL){
					httpcCloseContext(&context);
					free(lastbuf);
					if(newurl!=NULL) free(newurl);
					return -1;
				}
			}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);

	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		free(buf);
		return -1;
	}

	// Resize the buffer back down to our actual final size
	lastbuf = buf;
	buf = realloc(buf, size);
	if(buf==NULL){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
		if(newurl!=NULL) free(newurl);
		return -1;
	}
	httpcCloseContext(&context);
	if (newurl!=NULL) free(newurl);

	*response_data = malloc((strlen(buf) + 1) * sizeof(char));
	strcpy(*response_data, buf);
	free(buf);
	return 0;
}

void update_values() {
	Result ret=0;
	char *cryptocompare_response;

 	ret = http_get("https://min-api.cryptocompare.com/data/pricemultifull?fsyms=BTC,ETH,DASH,LTC,XMR&tsyms=USD,EUR,GBP,JPY,CNY", &cryptocompare_response);

	cJSON *root = cJSON_Parse(cryptocompare_response);
 	cJSON *raw = cJSON_GetObjectItem(root, "RAW");

 	for (int i = 0; i < 5; i++) {
 		cJSON *data = cJSON_GetObjectItem(raw, lst[i].sym);
 		for (int j = 0; j < 5; j++) {
 			cJSON *cur_data = cJSON_GetObjectItem(data, lst[i].currency[j].symbol);

			lst[i].currency[j].price = cJSON_GetObjectItem(cur_data, "PRICE")->valuedouble;
			lst[i].currency[j].high = cJSON_GetObjectItem(cur_data, "HIGH24HOUR")->valuedouble;
			lst[i].currency[j].low = cJSON_GetObjectItem(cur_data, "LOW24HOUR")->valuedouble;
			lst[i].currency[j].market_cap = cJSON_GetObjectItem(cur_data, "MKTCAP")->valuedouble;
			lst[i].currency[j].change_price = cJSON_GetObjectItem(cur_data, "CHANGEPCT24HOUR")->valuedouble;
			lst[i].currency[j].change_amt = cJSON_GetObjectItem(cur_data, "CHANGE24HOUR")->valuedouble;
			lst[i].currency[j].volume = cJSON_GetObjectItem(cur_data, "VOLUME24HOURTO")->valuedouble;
			lst[i].circulating = cJSON_GetObjectItem(cur_data, "SUPPLY")->valueint;
		}
	}
}

void initialize_values() {
	lst[0].full = "Bitcoin";
	lst[0].sym = "BTC";
	lst[1].full = "Dash";
	lst[1].sym = "DASH";
	lst[2].full = "Ethereum Hard Fork";
	lst[2].sym = "ETH";
	lst[3].full = "Litecoin";
	lst[3].sym = "LTC";
	lst[4].full = "Monero";
	lst[4].sym = "XMR";
	// really wanted to display the actual symbol but there seems to be a bug with sftdlib :(
	for (int i = 0; i < 5; i++) {
		lst[i].currency[0].symbol = "USD";
		lst[i].currency[1].symbol = "EUR";
		lst[i].currency[2].symbol = "GBP";
		lst[i].currency[3].symbol = "JPY";
		lst[i].currency[4].symbol = "CNY";
	}
}