/*
 * Copyright (C) 2020  Cumulus Networks, Inc.
 * Chirag Shah
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <zebra.h>
#include "northbound.h"
#include "libfrr.h"
#include "zebra_nb.h"
#include "zebra/interface.h"
#include "zebra/zebra_router.h"
#include "zebra/debug.h"
#include "printfrr.h"

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/up-count
 */
struct yang_data *
lib_interface_zebra_state_up_count_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct interface *ifp = args->list_entry;
	struct zebra_if *zebra_if;

	zebra_if = ifp->info;

	return yang_data_new_uint16(args->xpath, zebra_if->up_count);
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/down-count
 */
struct yang_data *
lib_interface_zebra_state_down_count_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct interface *ifp = args->list_entry;
	struct zebra_if *zebra_if;

	zebra_if = ifp->info;

	return yang_data_new_uint16(args->xpath, zebra_if->down_count);
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/zif-type
 */
struct yang_data *
lib_interface_zebra_state_zif_type_get_elem(struct nb_cb_get_elem_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/ptm-status
 */
struct yang_data *
lib_interface_zebra_state_ptm_status_get_elem(struct nb_cb_get_elem_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/vlan-id
 */
struct yang_data *
lib_interface_zebra_state_vlan_id_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct interface *ifp = args->list_entry;
	struct zebra_if *zebra_if;
	struct zebra_l2info_vlan *vlan_info;

	if (!IS_ZEBRA_IF_VLAN(ifp))
		return NULL;

	zebra_if = ifp->info;
	vlan_info = &zebra_if->l2info.vl;

	return yang_data_new_uint16(args->xpath, vlan_info->vid);
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/vni-id
 */
struct yang_data *
lib_interface_zebra_state_vni_id_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct interface *ifp = args->list_entry;
	struct zebra_if *zebra_if;
	struct zebra_l2info_vxlan *vxlan_info;

	if (!IS_ZEBRA_IF_VXLAN(ifp))
		return NULL;

	zebra_if = ifp->info;
	vxlan_info = &zebra_if->l2info.vxl;

	return yang_data_new_uint32(args->xpath, vxlan_info->vni);
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/remote-vtep
 */
struct yang_data *
lib_interface_zebra_state_remote_vtep_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct interface *ifp = args->list_entry;
	struct zebra_if *zebra_if;
	struct zebra_l2info_vxlan *vxlan_info;

	if (!IS_ZEBRA_IF_VXLAN(ifp))
		return NULL;

	zebra_if = ifp->info;
	vxlan_info = &zebra_if->l2info.vxl;

	return yang_data_new_ipv4(args->xpath, &vxlan_info->vtep_ip);
}

/*
 * XPath: /frr-interface:lib/interface/frr-zebra:zebra/state/mcast-group
 */
struct yang_data *
lib_interface_zebra_state_mcast_group_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct interface *ifp = args->list_entry;
	struct zebra_if *zebra_if;
	struct zebra_l2info_vxlan *vxlan_info;

	if (!IS_ZEBRA_IF_VXLAN(ifp))
		return NULL;

	zebra_if = ifp->info;
	vxlan_info = &zebra_if->l2info.vxl;

	return yang_data_new_ipv4(args->xpath, &vxlan_info->mcast_grp);
}

const void *lib_vrf_zebra_ribs_rib_get_next(struct nb_cb_get_next_args *args)
{
	struct vrf *vrf = (struct vrf *)args->parent_list_entry;
	struct zebra_router_table *zrt =
		(struct zebra_router_table *)args->list_entry;

	struct zebra_vrf *zvrf;
	afi_t afi;
	safi_t safi;

	zvrf = zebra_vrf_lookup_by_id(vrf->vrf_id);

	if (args->list_entry == NULL) {
		afi = AFI_IP;
		safi = SAFI_UNICAST;

		zrt = zebra_router_find_zrt(zvrf, zvrf->table_id, afi, safi);
		if (zrt == NULL)
			return NULL;
	} else {
		zrt = RB_NEXT(zebra_router_table_head, zrt);
		/* vrf_id/ns_id do not match, only walk for the given VRF */
		while (zrt && zrt->ns_id != zvrf->zns->ns_id)
			zrt = RB_NEXT(zebra_router_table_head, zrt);
	}

	return zrt;
}

int lib_vrf_zebra_ribs_rib_get_keys(struct nb_cb_get_keys_args *args)
{
	const struct zebra_router_table *zrt = args->list_entry;

	args->keys->num = 2;

	snprintfrr(args->keys->key[0], sizeof(args->keys->key[0]), "%s:%s",
		   "frr-zebra",
		   zebra_afi_safi_value2identity(zrt->afi, zrt->safi));
	snprintfrr(args->keys->key[1], sizeof(args->keys->key[1]), "%" PRIu32,
		   zrt->tableid);

	return NB_OK;
}

const void *
lib_vrf_zebra_ribs_rib_lookup_entry(struct nb_cb_lookup_entry_args *args)
{
	struct vrf *vrf = (struct vrf *)args->parent_list_entry;
	struct zebra_vrf *zvrf;
	afi_t afi;
	safi_t safi;
	uint32_t table_id = 0;

	zvrf = zebra_vrf_lookup_by_id(vrf->vrf_id);

	zebra_afi_safi_identity2value(args->keys->key[0], &afi, &safi);
	table_id = yang_str2uint32(args->keys->key[1]);
	/* table_id 0 assume vrf's table_id. */
	if (!table_id)
		table_id = zvrf->table_id;

	return zebra_router_find_zrt(zvrf, table_id, afi, safi);
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route
 */
const void *
lib_vrf_zebra_ribs_rib_route_get_next(struct nb_cb_get_next_args *args)
{
	const struct zebra_router_table *zrt = args->parent_list_entry;
	const struct route_node *rn = args->list_entry;

	if (args->list_entry == NULL)
		rn = route_top(zrt->table);
	else
		rn = srcdest_route_next((struct route_node *)rn);

	return rn;
}

int lib_vrf_zebra_ribs_rib_route_get_keys(struct nb_cb_get_keys_args *args)
{
	const struct route_node *rn = args->list_entry;

	args->keys->num = 1;
	prefix2str(&rn->p, args->keys->key[0], sizeof(args->keys->key[0]));

	return NB_OK;
}

const void *
lib_vrf_zebra_ribs_rib_route_lookup_entry(struct nb_cb_lookup_entry_args *args)
{
	const struct zebra_router_table *zrt = args->parent_list_entry;
	struct prefix p;
	struct route_node *rn;

	yang_str2prefix(args->keys->key[0], &p);

	rn = route_node_lookup(zrt->table, &p);

	if (!rn)
		return NULL;

	route_unlock_node(rn);

	return rn;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/prefix
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_prefix_get_elem(struct nb_cb_get_elem_args *args)
{
	const struct route_node *rn = args->list_entry;

	return yang_data_new_prefix(args->xpath, &rn->p);
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry
 */
const void *lib_vrf_zebra_ribs_rib_route_route_entry_get_next(
	struct nb_cb_get_next_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;
	struct route_node *rn = (struct route_node *)args->parent_list_entry;

	if (args->list_entry == NULL)
		RNODE_FIRST_RE(rn, re);
	else
		RNODE_NEXT_RE(rn, re);

	return re;
}

int lib_vrf_zebra_ribs_rib_route_route_entry_get_keys(
	struct nb_cb_get_keys_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	args->keys->num = 1;

	strlcpy(args->keys->key[0], zebra_route_string(re->type),
		sizeof(args->keys->key[0]));

	return NB_OK;
}

const void *lib_vrf_zebra_ribs_rib_route_route_entry_lookup_entry(
	struct nb_cb_lookup_entry_args *args)
{
	struct route_node *rn = (struct route_node *)args->parent_list_entry;
	struct route_entry *re = NULL;
	int proto_type = 0;
	afi_t afi;

	afi = family2afi(rn->p.family);
	proto_type = proto_redistnum(afi, args->keys->key[0]);

	RNODE_FOREACH_RE (rn, re) {
		if (proto_type == re->type)
			return re;
	}

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/protocol
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_protocol_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	return yang_data_new_enum(args->xpath, re->type);
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/instance
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_instance_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (re->instance)
		return yang_data_new_uint16(args->xpath, re->instance);

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/distance
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_distance_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	return yang_data_new_uint8(args->xpath, re->distance);
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/metric
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_metric_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	return yang_data_new_uint32(args->xpath, re->metric);
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/tag
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_tag_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (re->tag)
		return yang_data_new_uint32(args->xpath, re->tag);

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/selected
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_selected_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (CHECK_FLAG(re->flags, ZEBRA_FLAG_SELECTED))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/installed
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_installed_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (CHECK_FLAG(re->status, ROUTE_ENTRY_INSTALLED))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/failed
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_failed_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (CHECK_FLAG(re->status, ROUTE_ENTRY_FAILED))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/queued
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_queued_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (CHECK_FLAG(re->status, ROUTE_ENTRY_QUEUED))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/internal-flags
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_internal_flags_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (re->flags)
		return yang_data_new_int32(args->xpath, re->flags);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/internal-status
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_internal_status_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	if (re->status)
		return yang_data_new_int32(args->xpath, re->status);

	return NULL;
}

/*
 * XPath: /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/uptime
 */
struct yang_data *lib_vrf_zebra_ribs_rib_route_route_entry_uptime_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct route_entry *re = (struct route_entry *)args->list_entry;

	return yang_data_new_date_and_time(args->xpath, re->uptime);
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group
 */
const void *lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_get_next(
	struct nb_cb_get_next_args *args)
{
	struct route_entry *re = (struct route_entry *)args->parent_list_entry;
	struct nhg_hash_entry *nhe = (struct nhg_hash_entry *)args->list_entry;

	if (nhe == NULL) {
		nhe = re->nhe;
		return nhe;
	}
	return NULL;
}

int lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_get_keys(
	struct nb_cb_get_keys_args *args)
{
	struct nhg_hash_entry *nhe = (struct nhg_hash_entry *)args->list_entry;

	args->keys->num = 1;
	snprintfrr(args->keys->key[0], sizeof(args->keys->key[0]), "%" PRIu32,
		   nhe->id);

	return NB_OK;
}

const void *lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_lookup_entry(
	struct nb_cb_lookup_entry_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/name
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_name_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nhg_hash_entry *nhe = (struct nhg_hash_entry *)args->list_entry;
	char name[20] = {'\0'};

	snprintfrr(name, sizeof(name), "%" PRIu32, nhe->id);

	return yang_data_new_string(args->xpath, name);
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop
 */
const void *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_get_next(
	struct nb_cb_get_next_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;
	struct nhg_hash_entry *nhe =
		(struct nhg_hash_entry *)args->parent_list_entry;

	if (args->list_entry == NULL)
		nexthop = nhe->nhg.nexthop;
	else
		nexthop = nexthop_next(nexthop);

	return nexthop;
}

int lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_get_keys(
	struct nb_cb_get_keys_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	args->keys->num = 3;

	strlcpy(args->keys->key[0], yang_nexthop_type2str(nexthop->type),
		sizeof(args->keys->key[0]));

	switch (nexthop->type) {
	case NEXTHOP_TYPE_IPV4:
	case NEXTHOP_TYPE_IPV4_IFINDEX:
		snprintfrr(args->keys->key[1], sizeof(args->keys->key[1]),
			   "%pI4", &nexthop->gate.ipv4);
		if (nexthop->ifindex)
			strlcpy(args->keys->key[2],
				ifindex2ifname(nexthop->ifindex,
					       nexthop->vrf_id),
				sizeof(args->keys->key[2]));
		else
			/* no ifindex */
			strlcpy(args->keys->key[2], " ",
				sizeof(args->keys->key[2]));

		break;
	case NEXTHOP_TYPE_IPV6:
	case NEXTHOP_TYPE_IPV6_IFINDEX:
		snprintfrr(args->keys->key[1], sizeof(args->keys->key[1]),
			   "%pI6", &nexthop->gate.ipv6);

		if (nexthop->ifindex)
			strlcpy(args->keys->key[2],
				ifindex2ifname(nexthop->ifindex,
					       nexthop->vrf_id),
				sizeof(args->keys->key[2]));
		else
			/* no ifindex */
			strlcpy(args->keys->key[2], " ",
				sizeof(args->keys->key[2]));

		break;
	case NEXTHOP_TYPE_IFINDEX:
		strlcpy(args->keys->key[1], "", sizeof(args->keys->key[1]));
		strlcpy(args->keys->key[2],
			ifindex2ifname(nexthop->ifindex, nexthop->vrf_id),
			sizeof(args->keys->key[2]));

		break;
	case NEXTHOP_TYPE_BLACKHOLE:
		/* Gateway IP */
		strlcpy(args->keys->key[1], "", sizeof(args->keys->key[1]));
		strlcpy(args->keys->key[2], " ", sizeof(args->keys->key[2]));
		break;
	default:
		break;
	}

	return NB_OK;
}

const void *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_lookup_entry(
	struct nb_cb_lookup_entry_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/nh-type
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_nh_type_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	switch (nexthop->type) {
	case NEXTHOP_TYPE_IFINDEX:
		return yang_data_new_string(args->xpath, "ifindex");
		break;
	case NEXTHOP_TYPE_IPV4:
		return yang_data_new_string(args->xpath, "ip4");
		break;
	case NEXTHOP_TYPE_IPV4_IFINDEX:
		return yang_data_new_string(args->xpath, "ip4-ifindex");
		break;
	case NEXTHOP_TYPE_IPV6:
		return yang_data_new_string(args->xpath, "ip6");
		break;
	case NEXTHOP_TYPE_IPV6_IFINDEX:
		return yang_data_new_string(args->xpath, "ip6-ifindex");
		break;
	default:
		break;
	}

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/vrf
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_vrf_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	return yang_data_new_string(args->xpath,
				    vrf_id_to_name(nexthop->vrf_id));
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/gateway
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_gateway_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;
	struct ipaddr addr;

	switch (nexthop->type) {
	case NEXTHOP_TYPE_IPV4:
	case NEXTHOP_TYPE_IPV4_IFINDEX:
		addr.ipa_type = IPADDR_V4;
		memcpy(&addr.ipaddr_v4, &(nexthop->gate.ipv4),
		       sizeof(struct in_addr));
		break;
	case NEXTHOP_TYPE_IPV6:
	case NEXTHOP_TYPE_IPV6_IFINDEX:
		addr.ipa_type = IPADDR_V6;
		memcpy(&addr.ipaddr_v6, &(nexthop->gate.ipv6),
		       sizeof(struct in6_addr));
		break;
	case NEXTHOP_TYPE_BLACKHOLE:
	case NEXTHOP_TYPE_IFINDEX:
		/* No addr here */
		return yang_data_new_string(args->xpath, "");
		break;
	default:
		break;
	}

	return yang_data_new_ip(args->xpath, &addr);
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/interface
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_interface_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (nexthop->ifindex)
		yang_data_new_string(
			args->xpath,
			ifindex2ifname(nexthop->ifindex, nexthop->vrf_id));

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/bh-type
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_bh_type_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;
	const char *type_str = "";

	if (nexthop->type != NEXTHOP_TYPE_BLACKHOLE)
		return NULL;

	switch (nexthop->bh_type) {
	case BLACKHOLE_NULL:
		type_str = "null";
		break;
	case BLACKHOLE_REJECT:
		type_str = "reject";
		break;
	case BLACKHOLE_ADMINPROHIB:
		type_str = "prohibited";
		break;
	case BLACKHOLE_UNSPEC:
		type_str = "unspec";
		break;
	}

	return yang_data_new_string(args->xpath, type_str);
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/onlink
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_onlink_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ONLINK))
		return yang_data_new_bool(args->xpath, true);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/mpls-label-stack/entry
 */
const void *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_get_next(
	struct nb_cb_get_next_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

int lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_get_keys(
	struct nb_cb_get_keys_args *args)
{
	/* TODO: implement me. */
	return NB_OK;
}

const void *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_lookup_entry(
	struct nb_cb_lookup_entry_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/mpls-label-stack/entry/id
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_id_get_elem(
	struct nb_cb_get_elem_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/mpls-label-stack/entry/label
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_label_get_elem(
	struct nb_cb_get_elem_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/mpls-label-stack/entry/ttl
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_ttl_get_elem(
	struct nb_cb_get_elem_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/mpls-label-stack/entry/traffic-class
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_mpls_label_stack_entry_traffic_class_get_elem(
	struct nb_cb_get_elem_args *args)
{
	/* TODO: implement me. */
	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/duplicate
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_duplicate_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_DUPLICATE))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/recursive
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_recursive_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/active
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_active_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/fib
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_fib_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
		return yang_data_new_empty(args->xpath);

	return NULL;
}

/*
 * XPath:
 * /frr-vrf:lib/vrf/frr-zebra:zebra/ribs/rib/route/route-entry/nexthop-group/frr-nexthops/nexthop/weight
 */
struct yang_data *
lib_vrf_zebra_ribs_rib_route_route_entry_nexthop_group_frr_nexthops_nexthop_weight_get_elem(
	struct nb_cb_get_elem_args *args)
{
	struct nexthop *nexthop = (struct nexthop *)args->list_entry;

	if (nexthop->weight)
		return yang_data_new_uint8(args->xpath, nexthop->weight);

	return NULL;
}
