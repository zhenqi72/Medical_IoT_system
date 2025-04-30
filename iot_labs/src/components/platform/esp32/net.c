#include "net.h"
#include "os_func.h"
#include "../util/utilities.h"
#include "netdb.h"

uint64_t plt_net_htonll(uint64_t n)
{
	uint32_t high = n >> 32;
	uint32_t low = n;

	return (((uint64_t)htonl(low)) << 32 ) | htonl(high);

}

uint64_t plt_net_ntohll(uint64_t n)
{
	return plt_net_htonll(n);
}

static char * _net_addr_ntoa(uint32_t addr)
{
  static char str[64] = {0};
  snprintf_sf(str, sizeof(str), "%d.%d.%d.%d", (addr>>0)&0xff, (addr>>8)&0xff, (addr>>16)&0xff, (addr>>24)&0xff);
  return str;
}

char *plt_net_htoa(net_addr_t* paddr)
{
	return _net_addr_ntoa(htonl(paddr->s_addr));
}

int plt_net_htoa_buf(net_addr_t* paddr, char* buf, int buf_len)
{
	int len;
	uint32_t addr = paddr->s_addr;
	len = snprintf_sf(buf, buf_len, "%d.%d.%d.%d", (addr>>24)&0xff, (addr>>16)&0xff, (addr>>8)&0xff, (addr>>0)&0xff);

	return len;
}

int plt_net_name2ip(const char *name, net_addr_t *addr)
{

	struct hostent hostbuf, *hp;
	size_t hstbuflen;
	char *tmphstbuf;
	int res;
	int herr;

	hstbuflen = 1024;

	/* Allocate buffer, remember to free it to avoid memory leakage.  */
	tmphstbuf = malloc (hstbuflen);

	if(NULL == tmphstbuf){
		return -1;
	}

	while ((res = gethostbyname_r (name, &hostbuf, tmphstbuf, hstbuflen, &hp, &herr)) == ERANGE){

		/* Enlarge the buffer.  */
		hstbuflen *= 2;

		tmphstbuf = realloc (tmphstbuf, hstbuflen);

		if(NULL == tmphstbuf){
			return -1;
		}

	}

	/*  Check for errors.  */
	if (0 == res && hp){

		addr->s_addr = ntohl(*((uint32_t*)(hp->h_addr_list[0])));

		free(tmphstbuf);

		return 0;
	}


	free(tmphstbuf);

	return -1;

}


uint32_t plt_net_ip2n(const char* ip)
{
	struct in_addr addr = {0};
	if(0 != inet_aton(ip, &addr)){
		return addr.s_addr;
	}
	return 0;
}

int plt_net_get_ip_info(ip_info_t *ip_info)
{
	tcpip_adapter_if_t esp_if;
	tcpip_adapter_ip_info_t esp_ip_info;

	wifi_mode_t wifi_mode = WIFI_MODE_STA;

	esp_wifi_get_mode(&wifi_mode);

	if(WIFI_MODE_AP == wifi_mode){
		esp_if = TCPIP_ADAPTER_IF_AP;
	}
	else{
		esp_if = TCPIP_ADAPTER_IF_STA;
	}

	if(ESP_OK != tcpip_adapter_get_ip_info(esp_if, &esp_ip_info))
		return -1;

	ip_info->ip.s_addr = ntohl(esp_ip_info.ip.addr);
	ip_info->netmask.s_addr = ntohl(esp_ip_info.netmask.addr);
	ip_info->gateway.s_addr = ntohl(esp_ip_info.gw.addr);

	return 0;
}

bool plt_net_is_offline(void)
{
	tcpip_adapter_ip_info_t esp_ip_info;

	if(ESP_OK != tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &esp_ip_info))
		return true;

	if (esp_ip_info.ip.addr != 0) {
		return false;
	}
	else{
		return true;
	}
}

int plt_net_get_ap_info(net_wifi_ap_info_t *ap_info)
{
	wifi_ap_record_t ap_record;
	
	if(ESP_OK == esp_wifi_sta_get_ap_info(&ap_record)){

		memcpy(ap_info->ssid, ap_record.ssid, sizeof(ap_info->ssid));
		ap_info->ssid[sizeof(ap_info->ssid)-1] = '\0';
		memcpy(ap_info->bssid, ap_record.bssid, sizeof(ap_info->bssid));
		ap_info->channel = ap_record.primary;
		ap_info->rssi = ap_record.rssi;

		return 0;
	}

	return -1;

}
