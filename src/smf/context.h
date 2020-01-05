/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SMF_CONTEXT_H
#define SMF_CONTEXT_H

#include "smf-config.h"

#if HAVE_NET_IF_H
#include <net/if.h>
#endif

#include "ogs-gtp.h"
#include "ogs-diameter-gx.h"
#include "ogs-pfcp.h"
#include "ogs-app.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_OF_DEV          16
#define MAX_NUM_OF_SUBNET       16

extern int __smf_log_domain;

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __smf_log_domain

typedef struct smf_context_s {
    const char*         diam_conf_path;   /* SMF Diameter conf path */
    ogs_diam_config_t   *diam_config;     /* SMF Diameter config */

    uint32_t        gtpc_port;      /* Default: SMF GTP-C local port */
    uint32_t        pfcp_port;      /* Default: SMF GTP-U local port */
    const char      *tun_ifname;    /* Default: ogstun */

    ogs_list_t      gtpc_list;      /* SMF GTPC IPv4 Server List */
    ogs_list_t      gtpc_list6;     /* SMF GTPC IPv6 Server List */
    ogs_sock_t      *gtpc_sock;     /* SMF GTPC IPv4 Socket */
    ogs_sock_t      *gtpc_sock6;    /* SMF GTPC IPv6 Socket */
    ogs_sockaddr_t  *gtpc_addr;     /* SMF GTPC IPv4 Address */
    ogs_sockaddr_t  *gtpc_addr6;    /* SMF GTPC IPv6 Address */

    ogs_list_t      pfcp_list;      /* SMF PFCP IPv4 Server List */
    ogs_list_t      pfcp_list6;     /* SMF PFCP IPv6 Server List */
    ogs_sock_t      *pfcp_sock;     /* SMF PFCP IPv4 Socket */
    ogs_sock_t      *pfcp_sock6;    /* SMF PFCP IPv6 Socket */
    ogs_sockaddr_t  *pfcp_addr;     /* SMF PFCP IPv4 Address */
    ogs_sockaddr_t  *pfcp_addr6;    /* SMF PFCP IPv6 Address */

    uint32_t        pfcp_started;   /* UTC time when the PFCP entity started */
    uint8_t         cp_function_features; /* CP Function Features */

    ogs_list_t      dev_list;       /* SMF Tun Device List */
    ogs_list_t      subnet_list;    /* SMF UE Subnet List */

    ogs_queue_t     *queue;         /* Queue for processing SMF control */
    ogs_timer_mgr_t *timer_mgr;     /* Timer Manager */
    ogs_pollset_t   *pollset;       /* Poll Set for I/O Multiplexing */

#define MAX_NUM_OF_DNS              2
    const char      *dns[MAX_NUM_OF_DNS];
    const char      *dns6[MAX_NUM_OF_DNS];

#define MAX_NUM_OF_P_CSCF           16
    const char      *p_cscf[MAX_NUM_OF_P_CSCF];
    int             num_of_p_cscf;
    int             p_cscf_index;
    const char      *p_cscf6[MAX_NUM_OF_P_CSCF];
    int             num_of_p_cscf6;
    int             p_cscf6_index;

    ogs_list_t      sgw_s5c_list;   /* SGW GTPC Node List */
    ogs_list_t      sgw_s5u_list;   /* SGW GTPU Node List */
    ogs_list_t      ip_pool_list;

    ogs_list_t      upf_n4_list;    /* UPF PFCP Node List */
    ogs_pfcp_node_t *upf;           /* Iterator for UPF round-robin */

    ogs_hash_t      *sess_hash;     /* hash table (IMSI+APN) */

    ogs_list_t      sess_list;
} smf_context_t;

typedef struct smf_subnet_s smf_subnet_t;
typedef struct smf_ue_ip_s {
    uint32_t        addr[4];
    bool            static_ip;

    /* Related Context */
    smf_subnet_t    *subnet;
} smf_ue_ip_t;

typedef struct smf_dev_s {
    ogs_lnode_t     lnode;

    char            ifname[IFNAMSIZ];
    ogs_socket_t    fd;

    ogs_sockaddr_t  *link_local_addr;
    ogs_poll_t      *poll;
} smf_dev_t;

typedef struct smf_subnet_s {
    ogs_lnode_t     node;

    ogs_ipsubnet_t  sub;                /* Subnet : cafe::0/64 */
    ogs_ipsubnet_t  gw;                 /* Gateway : cafe::1 */
    char            apn[OGS_MAX_APN_LEN];   /* APN : "internet", "volte", .. */

#define MAX_NUM_OF_SUBNET_RANGE         16
    struct {
        const char *low;
        const char *high;
    } range[MAX_NUM_OF_SUBNET_RANGE];
    int num_of_range;

    int             family;         /* AF_INET or AF_INET6 */
    uint8_t         prefixlen;      /* prefixlen */
    OGS_POOL(pool, smf_ue_ip_t);

    smf_dev_t       *dev;           /* Related Context */
} smf_subnet_t;

typedef struct smf_sess_s {
    ogs_lnode_t     lnode;
    uint32_t        index;          /**< An index of this node */

    uint32_t        smf_s5c_teid;   /* SMF-S5C-TEID is derived from INDEX */
    uint32_t        sgw_s5c_teid;   /* SGW-S5C-TEID is received from SGW */

    char            *gx_sid;        /* Gx Session ID */

    uint64_t        smf_n4_seid;    /* SMF SEID is dervied from INDEX */
    uint64_t        upf_n4_seid;    /* UPF SEID is received from UPF */

    /* IMSI */
    uint8_t         imsi[OGS_MAX_IMSI_LEN];
    int             imsi_len;
    char            imsi_bcd[OGS_MAX_IMSI_BCD_LEN+1];

    /* APN Configuration */
    ogs_pdn_t       pdn;
    smf_ue_ip_t     *ipv4;
    smf_ue_ip_t     *ipv6;

    /* User-Lication-Info */
    ogs_tai_t       tai;
    ogs_e_cgi_t     e_cgi;

    uint8_t         hash_keybuf[OGS_MAX_IMSI_LEN+OGS_MAX_APN_LEN+1];
    int             hash_keylen;

    ogs_tlv_octet_t ue_pco; /* Save Protocol Configuration Options from UE */
    ogs_tlv_octet_t ue_timezone; /* UE Timezone */

    ogs_list_t      bearer_list;

    /* PDR ID Generator(1~MAX_NUM_OF_PDR) */
    uint8_t         pdr_id;
    ogs_list_t      pdr_list;

    /* Related Context */
    ogs_gtp_node_t  *gnode;
    ogs_pfcp_node_t *pnode;
} smf_sess_t;

typedef struct smf_bearer_s {
    ogs_lnode_t     lnode; /**< A node of list_t */
    uint32_t        index;

    uint8_t         ebi;

    uint32_t        smf_s5u_teid;   /* SMF_S5U is derived from INDEX */
    uint32_t        sgw_s5u_teid;   /* SGW_S5U is received from SGW */

    char            *name;          /* PCC Rule Name */
    ogs_qos_t       qos;            /* QoS Infomration */

    /* Packet Filter Identifier Generator(1~15) */
    uint8_t         pf_identifier;
    /* Packet Filter List */
    ogs_list_t      pf_list;

    smf_sess_t      *sess;
    ogs_gtp_node_t  *gnode;
} smf_bearer_t;

typedef struct smf_pdr_s {
    ogs_lnode_t     lnode;

    uint16_t        id;

    /* FAR ID Generator(1~MAX_NUM_OF_FAR) */
    uint8_t         far_id;
    ogs_list_t      far_list;

    /* URR ID Generator(1~MAX_NUM_OF_URR) */
    uint8_t         urr_id;
    ogs_list_t      urr_list;

    /* QER ID Generator(1~MAX_NUM_OF_QER) */
    uint8_t         qer_id;
    ogs_list_t      qer_list;

    smf_sess_t      *sess;
} smf_pdr_t;

typedef struct smf_far_s {
    ogs_lnode_t     lnode;

    uint16_t        id;

    smf_pdr_t       *pdr;
} smf_far_t;

typedef struct smf_urr_s {
    ogs_lnode_t     lnode;

    uint16_t        id;

    smf_pdr_t       *pdr;
} smf_urr_t;

typedef struct smf_qer_s {
    ogs_lnode_t     lnode;

    uint16_t        id;

    smf_pdr_t       *pdr;
} smf_qer_t;

typedef struct smf_rule_s {
    uint8_t proto;
ED5(uint8_t ipv4_local:1;,
    uint8_t ipv4_remote:1;,
    uint8_t ipv6_local:1;,
    uint8_t ipv6_remote:1;,
    uint8_t reserved:4;)
    struct {
        struct {
            uint32_t addr[4];
            uint32_t mask[4];
        } local;
        struct {
            uint32_t addr[4];
            uint32_t mask[4];
        } remote;
    } ip;
    struct {
        struct {
            uint16_t low;
            uint16_t high;
        } local;
        struct {
            uint16_t low;
            uint16_t high;
        } remote;
    } port;
} smf_rule_t;

typedef struct smf_pf_s {
    ogs_lnode_t     lnode;

ED3(uint8_t spare:2;,
    uint8_t direction:2;,
    uint8_t identifier:4;)
    smf_rule_t      rule;

    smf_bearer_t    *bearer;
} smf_pf_t;

void smf_context_init(void);
void smf_context_final(void);
smf_context_t *smf_self(void);

int smf_context_parse_config(void);

smf_sess_t *smf_sess_add_by_message(ogs_gtp_message_t *message);

smf_sess_t *smf_sess_add(
        uint8_t *imsi, int imsi_len, char *apn,
        uint8_t pdn_type, uint8_t ebi, ogs_paa_t *addr);

int smf_sess_remove(smf_sess_t *sess);
void smf_sess_remove_all(void);
smf_sess_t *smf_sess_find(uint32_t index);
smf_sess_t *smf_sess_find_by_teid(uint32_t teid);
smf_sess_t *smf_sess_find_by_seid(uint64_t seid);
smf_sess_t *smf_sess_find_by_imsi_apn(uint8_t *imsi, int imsi_len, char *apn);

smf_bearer_t *smf_bearer_add(smf_sess_t *sess);
int smf_bearer_remove(smf_bearer_t *bearer);
void smf_bearer_remove_all(smf_sess_t *sess);
smf_bearer_t *smf_bearer_find(uint32_t index);
smf_bearer_t *smf_bearer_find_by_smf_s5u_teid(uint32_t smf_s5u_teid);
smf_bearer_t *smf_bearer_find_by_ebi(smf_sess_t *sess, uint8_t ebi);
smf_bearer_t *smf_bearer_find_by_name(smf_sess_t *sess, char *name);
smf_bearer_t *smf_bearer_find_by_qci_arp(smf_sess_t *sess, 
                                uint8_t qci,
                                uint8_t priority_level,
                                uint8_t pre_emption_capability,
                                uint8_t pre_emption_vulnerability);
smf_bearer_t *smf_default_bearer_in_sess(smf_sess_t *sess);
smf_bearer_t *smf_bearer_first(smf_sess_t *sess);
smf_bearer_t *smf_bearer_next(smf_bearer_t *bearer);

smf_pdr_t *smf_pdr_add(smf_sess_t *sess);
int smf_pdr_remove(smf_pdr_t *pdr);
void smf_pdr_remove_all(smf_sess_t *sess);
smf_pdr_t *smf_pdr_find_by_id(smf_sess_t *sess, uint8_t id);

smf_pf_t *smf_pf_add(smf_bearer_t *bearer, uint32_t precedence);
int smf_pf_remove(smf_pf_t *pf);
void smf_pf_remove_all(smf_bearer_t *bearer);
smf_pf_t *smf_pf_find_by_id(smf_bearer_t *smf_bearer, uint8_t id);
smf_pf_t *smf_pf_first(smf_bearer_t *bearer);
smf_pf_t *smf_pf_next(smf_pf_t *pf);

int smf_ue_pool_generate(void);
smf_ue_ip_t *smf_ue_ip_alloc(int family, const char *apn, uint8_t *addr);
int smf_ue_ip_free(smf_ue_ip_t *ip);

smf_dev_t *smf_dev_add(const char *ifname);
int smf_dev_remove(smf_dev_t *dev);
void smf_dev_remove_all(void);
smf_dev_t *smf_dev_find_by_ifname(const char *ifname);
smf_dev_t *smf_dev_first(void);
smf_dev_t *smf_dev_next(smf_dev_t *dev);

smf_subnet_t *smf_subnet_add(
        const char *ipstr, const char *mask_or_numbits,
        const char *apn, const char *ifname);
smf_subnet_t *smf_subnet_next(smf_subnet_t *subnet);
int smf_subnet_remove(smf_subnet_t *subnet);
void smf_subnet_remove_all(void);
smf_subnet_t *smf_subnet_first(void);
smf_subnet_t *gw_subnet_next(smf_subnet_t *subnet);

void stats_add_session(void);
void stats_remove_session(void);

#ifdef __cplusplus
}
#endif

#endif /* SMF_CONTEXT_H */
