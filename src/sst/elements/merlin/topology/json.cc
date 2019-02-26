#include <sst/core/sst_config.h>
#include <sst/core/elementinfo.h>
#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/params.h>

#include "sst/elements/merlin/router.h"

namespace SST {
namespace Merlin {

class TopoJSON: public Topology {

public:

    SST_ELI_REGISTER_SUBCOMPONENT(
        TopoJSON,
        "merlin",
        "json",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "JSON topology object",
        "SST::Merlin::Topology")

    SST_ELI_DOCUMENT_PARAMS(
        {"json:table",  "List of ports where table[i] is the port to follow for node i"},
        {"json:rtr_ports", "Shape of the fattree"},
        {"json:node_ports", ""},
    )

private:
  std::vector<int> dst_to_port_;
  std::vector<int> port_to_dst_;
  std::set<int> rtr_ports_;
  std::set<int> node_ports_;

public:
    TopoJSON(Component* comp, Params& params) :
      Topology(comp)
    {
      params.find_array("json:table", dst_to_port_);

      if (params.contains("json:rtr_ports")){
        std::vector<int> rtr_ports;
        params.find_array("json:rtr_ports", rtr_ports);
        for (auto port : rtr_ports) rtr_ports_.insert(port);
      }

      if (params.contains("json:node_ports")){
        std::vector<int> node_ports;
        params.find_array("json:node_ports", node_ports);
        for (auto port : node_ports) node_ports_.insert(port);
      }


      std::map<int,int> inj_ports;
      int max_inj_port = 0;
      for (int dst=0; dst < dst_to_port_.size(); ++dst){
        int port = dst_to_port_[dst];
        bool is_inj_port = node_ports_.find(port) != node_ports_.end();
        if (is_inj_port){
          max_inj_port = std::max(max_inj_port, port);
          inj_ports[port] = dst;
        }
      }
      port_to_dst_.resize(max_inj_port+1);
      for (auto& pair : inj_ports){
        port_to_dst_[pair.first] = pair.second;
      }

    }

    ~TopoJSON(){}

    /**
     * @brief route
     * @param port input port
     * @param vc
     * @param ev
     */
    void route(int port, int vc, internal_router_event* ev) override {
      int dest = ev->getDest();
      ev->setNextPort(dst_to_port_[dest]);
    }

    internal_router_event* process_input(RtrEvent* ev) override {
      internal_router_event* ire = new internal_router_event(ev);
      ire->setVC(ev->request->vn);
      return ire;
    }

    void routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts) override {
      if (ev->getDest() == INIT_BROADCAST_ADDR){
        //SST::Output::getDefaultObject().fatal(CALL_INFO, 1, "INIT_BROADCAST_ADDR unimplemented");
      } else {
        route(port, 0, ev);
        outPorts.push_back(ev->getNextPort());
      }
    }

    internal_router_event* process_InitData_input(RtrEvent* ev) override {
      auto* ret = new internal_router_event(ev);
      return ret;
    }

    int getEndpointID(int port) override {
      return port_to_dst_[port];
    }

    PortState getPortState(int port) const override {
      if (rtr_ports_.find(port) != rtr_ports_.end()){
        return R2R;
      } else if (node_ports_.find(port) != node_ports_.end()){
        return R2N;
      } else {
        return UNCONNECTED;
      }
    }

    int computeNumVCs(int vns) override {return vns;}

};

}
}
