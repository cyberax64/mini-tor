#include "consensus.h"
#include "parsers/consensus_parser.h"

#include <mini/logger.h>
#include <mini/io/file.h>
#include <mini/net/http.h>
#include <mini/crypto/random.h>

namespace mini::tor {

//
// static constexpr char* authorities[] = {
//   "moria1 orport=9101 v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
//   "tor26 orport=443 v3ident=14C131DFC5C6F93646BE72FA1401C02A8DF2E8B4 ipv6=[2001:858:2:2:aabb:0:563b:1526]:443 86.59.21.38:80 847B 1F85 0344 D787 6491 A548 92F9 0493 4E4E B85D",
//   "dizum orport=443 v3ident=E8A9C45EDE6D711294FADF8E7951F4DE6CA56B58 194.109.206.212:80 7EA6 EAD6 FD83 083C 538F 4403 8BBF A077 587D D755",
//   "Tonga orport=443 bridge 82.94.251.203:80 4A0C CD2D DC79 9508 3D73 F5D6 6710 0C8A 5831 F16D",
//   "gabelmoo orport=443 v3ident=ED03BB616EB2F60BEC80151114BB25CEF515B226 ipv6=[2001:638:a000:4140::ffff:189]:443 131.188.40.189:80 F204 4413 DAC2 E02E 3D6B CF47 35A1 9BCA 1DE9 7281",
//   "dannenberg orport=443 v3ident=0232AF901C31A04EE9848595AF9BB7620D4C5B2E 193.23.244.244:80 7BE6 83E6 5D48 1413 21C5 ED92 F075 C553 64AC 7123",
//   "maatuska orport=80 v3ident=49015F787433103580E3B66A1707A00E60F2D15B ipv6=[2001:67c:289c::9]:80 171.25.193.9:443 BD6A 8292 55CB 08E6 6FBE 7D37 4836 3586 E46B 3810",
//   "Faravahar orport=443 v3ident=EFCBE720AB3A82B99F9E953CD5BF50F7EEFC7B97 154.35.175.225:80 CF6D 0AAF B385 BE71 B8E1 11FC 5CFF 4B47 9237 33BC",
//   "longclaw orport=443 v3ident=23D15D965BC35114467363C165C4F724B64B4F66 ipv6=[2620:13:4000:8000:60:f3ff:fea1:7cff]:443 199.254.238.52:80 74A9 1064 6BCE EFBC D2E8 74FC 1DC9 9743 0F96 8145",
// };
//

struct authority_onion_router
{
  constexpr authority_onion_router(
    const char* /* name */, // ignored
    const char* ip,
    uint16_t /* or_port */, // ignored
    uint16_t dir_port
    )
    : ip(net::ip_address::from_string(ip))
    , dir_port(dir_port)
  {

  }

  const net::ip_address ip;
  const uint16_t        dir_port;
};

//
// list of directory authority routers.
//
// Liste mise à jour des autorités de répertoire Tor (2025-05-22)
static constexpr stack_buffer<authority_onion_router, 10> default_authority_list = { {
  authority_onion_router( "moria1"    , "128.31.0.39"     , 9101 , 9131 ),
  authority_onion_router( "tor26"     , "217.196.147.77"  ,  443 ,   80 ),
  authority_onion_router( "dizum"     , "45.66.35.11"     ,  443 ,   80 ),
  authority_onion_router( "Serge"     , "66.111.2.131"    , 9001 , 9030 ),
  authority_onion_router( "gabelmoo"  , "131.188.40.189"  ,  443 ,   80 ),
  authority_onion_router( "dannenberg", "193.23.244.244"  ,  443 ,   80 ),
  authority_onion_router( "maatuska"  , "171.25.193.9"    ,   80 ,  443 ),
  authority_onion_router( "longclaw"  , "199.58.81.140"   ,  443 ,   80 ),
  authority_onion_router( "bastet"    , "204.13.164.118"  ,  443 ,   80 ),
  authority_onion_router( "faravahar" , "216.218.219.41"  ,  443 ,   80 ),
} };

consensus::consensus(
  const string_ref cached_consensus_path,
  bool force_download
  )
{
  create(cached_consensus_path, force_download);
}

consensus::~consensus(
  void
  )
{
  destroy();
}

void
consensus::create(
  const string_ref cached_consensus_path,
  bool force_download
  )
{
  string consensus_content;
  bool have_valid_consensus = false;

  //
  // if no path to the cached consensus file
  // was provided, we have to download it.
  //
  if (cached_consensus_path.is_empty() || !io::file::exists(cached_consensus_path))
  {
    force_download = true;
  }

  while (!have_valid_consensus)
  {
    if (force_download) {
      // Essayer d'abord le chemin standard
      mini_info("Downloading consensus from directory authority...");
      consensus_content = download_from_random_router("/tor/status-vote/current/consensus", true);
      
      // Si le téléchargement a échoué, essayer le chemin alternatif
      if (consensus_content.is_empty()) {
        mini_info("Trying alternative consensus path (compressed)...");
        consensus_content = download_from_random_router("/tor/status-vote/current/consensus.z", true);
        
        // Si toujours vide, essayer un autre chemin
        if (consensus_content.is_empty()) {
          mini_info("Trying another alternative consensus path (network status)...");
          consensus_content = download_from_random_router("/tor/status-vote/current/ns", true);
          
          // Si toujours vide, essayer un autre chemin
          if (consensus_content.is_empty()) {
            mini_info("Trying microdescriptor consensus path...");
            consensus_content = download_from_random_router("/tor/status-vote/current/consensus-microdesc", true);
          }
        }
      }
    } else {
      consensus_content = io::file::read_to_string(cached_consensus_path);
    }

    //
    // assume newly downloaded consensus as valid.
    //
    const bool reject_invalid = !force_download;
    parse_consensus(consensus_content, reject_invalid);

    //
    // consider force_download-ed consensus as valid.
    //
    have_valid_consensus = force_download || (_valid_until >= time::now());

    //
    // if the consensus is invalid, we have to download it anyway.
    //
    if (!have_valid_consensus)
    {
      force_download = true;
    }
  }

  //
  // save the consensus content, if the path was provided.
  //
  if (force_download && !cached_consensus_path.is_empty())
  {
    io::file::write_from_string(cached_consensus_path, consensus_content);
  }
}

void
consensus::destroy(
  void
  )
{
  for (auto&& onion_router : _onion_router_map)
  {
    delete onion_router.second;
  }
}

onion_router*
consensus::get_onion_router_by_name(
  const string_ref name
  ) const
{
  for (auto&& pair : _onion_router_map)
  {
    auto router = pair.second;

    if (name == router->get_name())
    {
      return router;
    }
  }

  return nullptr;
}

onion_router*
consensus::get_onion_router_by_identity_fingerprint(
  const byte_buffer_ref identity_fingerprint
  )
{
  return _onion_router_map[identity_fingerprint];
}

onion_router_list
consensus::get_onion_routers_by_criteria(
  const search_criteria& criteria
  ) const
{
  onion_router_list result;

  for (auto&& pair : _onion_router_map)
  {
    auto router = pair.second;

    if (!criteria.allowed_dir_ports.is_empty())
    {
      if (criteria.allowed_dir_ports.index_of(router->get_dir_port()) == collections::list<uint16_t>::not_found)
      {
        continue;
      }
    }

    if (!criteria.allowed_or_ports.is_empty())
    {
      if (criteria.allowed_or_ports.index_of(router->get_or_port()) == collections::list<uint16_t>::not_found)
      {
        continue;
      }
    }

    if (!criteria.forbidden_onion_routers.is_empty())
    {
      if (criteria.forbidden_onion_routers.index_of(router) != onion_router_list::not_found)
      {
        continue;
      }
    }

    if (criteria.flags != onion_router::status_flag::none)
    {
      if ((router->get_flags() & criteria.flags) != criteria.flags)
      {
        continue;
      }
    }

    result.add(router);
  }

  return result;
}

onion_router*
consensus::get_random_onion_router_by_criteria(
  const search_criteria& criteria
  ) const
{
  auto routers = get_onion_routers_by_criteria(criteria);

  // Vérifier que la liste n'est pas vide avant de générer un index aléatoire
  const size_type random_index = routers.get_size() > 0 ? 
    crypto::random_device.get_random(routers.get_size()) : 0;

  return !routers.is_empty()
    ? routers[random_index]
    : nullptr;
}

string
consensus::get_onion_router_descriptor(
  const byte_buffer_ref identity_fingerprint
  )
{
  return download_from_random_router("/tor/server/fp/" + crypto::base16::encode(identity_fingerprint));
}

//
// directories
//

tor::onion_router::status_flags
consensus::get_allowed_dir_flags(
  void
  ) const
{
  return _allowed_dir_flags;
}

void
consensus::set_allowed_dir_flags(
  tor::onion_router::status_flags allowed_dir_flags
  )
{
  _allowed_dir_flags = allowed_dir_flags;
}

const collections::list<uint16_t>&
consensus::get_allowed_dir_ports(
  void
  ) const
{
  return _allowed_dir_ports;
}

void
consensus::set_allowed_dir_ports(
  const collections::list<uint16_t>& allowed_dir_ports
  )
{
  _allowed_dir_ports = allowed_dir_ports;
}

size_type
consensus::get_max_try_count(
  void
  ) const
{
  return _max_try_count;
}

void
consensus::set_max_try_count(
  size_type max_try_count
  )
{
  _max_try_count = max_try_count;
}

string
consensus::download_from_random_router(
  const string_ref path,
  bool only_authorities
  )
{
  size_type try_count = 0;
  string result;

  do
  {
    result = download_from_random_router_impl(path, only_authorities);
  } while (++try_count < _max_try_count && result.is_empty());

  return result;
}

string
consensus::download_from_random_router_impl(
  const string_ref path,
  bool only_authorities
  )
{
  net::ip_address ip;
  uint16_t port;

  //
  // if the onion router map is empty,
  // we're stuck to authorities anyway.
  //
  if (only_authorities || _onion_router_map.is_empty())
  {
    // Vérifier que la liste d'autorités n'est pas vide
    if (default_authority_list.get_size() == 0) {
      return string();
    }
    
    const size_type random_index = crypto::random_device.get_random(default_authority_list.get_size());
    auto authority = default_authority_list[random_index];

    ip = authority.ip;
    port = authority.dir_port;
  }
  else
  {
    auto router = get_random_onion_router_by_criteria({
      _allowed_dir_ports, {}, {}, _allowed_dir_flags
    });

    ip = router->get_ip_address();
    port = router->get_dir_port();
  }

  mini_debug(
    "consensus::download_from_random_authority() [path: http://%s:%u%s]",
    ip.to_string().get_buffer(),
    port,
    path.get_buffer());

  return net::http::client::get(ip.to_string(), port, path);
}

void
consensus::parse_consensus(
  const string_ref consensus_content,
  bool reject_invalid
  )
{
  //
  // clear the map first.
  //
  _onion_router_map.clear();

  //
  // parse the consensus document.
  //
  consensus_parser parser;
  parser.parse(*this, consensus_content, reject_invalid);
}

}
