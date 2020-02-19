/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "l2cache_lru.h"

namespace debugger {

L2CacheLru::L2CacheLru(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_valid("i_req_valid"),
    i_req_type("i_req_type"),
    i_req_size("i_req_size"),
    i_req_prot("i_req_prot"),
    i_req_addr("i_req_addr"),
    i_req_wdata("i_req_wdata"),
    i_req_wstrb("i_req_wstrb"),
    o_req_ready("o_req_ready"),
    o_resp_valid("o_resp_valid"),
    o_resp_rdata("o_resp_rdata"),
    o_resp_status("o_resp_status"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_cached("o_req_mem_cached"),
    o_req_mem_size("o_req_mem_size"),
    o_req_mem_prot("o_req_mem_prot"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data("i_mem_data"),
    i_mem_data_ack("i_mem_data_ack"),
    i_mem_load_fault("i_mem_load_fault"),
    i_mem_store_fault("i_mem_store_fault"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    o_flush_end("o_flush_end") {
    async_reset_ = async_reset;

    mem = new TagMemNWay<CFG_CPU_ADDR_BITS,
                         CFG_L2_LOG2_NWAYS,
                         CFG_L2_LOG2_LINES_PER_WAY,
                         CFG_L2_LOG2_BYTES_PER_LINE,
                         L2TAG_FL_TOTAL,
                         0>("mem0", async_reset);
    mem->i_clk(i_clk);
    mem->i_nrst(i_nrst);
    mem->i_cs(line_cs_i);
    mem->i_flush(line_flush_i);
    mem->i_addr(line_addr_i);
    mem->i_wdata(line_wdata_i);
    mem->i_wstrb(line_wstrb_i);
    mem->i_wflags(line_wflags_i);
    mem->o_raddr(line_raddr_o);
    mem->o_rdata(line_rdata_o);
    mem->o_rflags(line_rflags_o);
    mem->o_hit(line_hit_o);
    mem->i_snoop_addr(line_snoop_addr_i);
    mem->o_snoop_ready(line_snoop_ready_o);
    mem->o_snoop_flags(line_snoop_flags_o);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_valid;
    sensitive << i_req_type;
    sensitive << i_req_size;
    sensitive << i_req_prot;
    sensitive << i_req_addr;
    sensitive << i_req_wdata;
    sensitive << i_req_wstrb;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_mem_data_ack;
    sensitive << i_mem_load_fault;
    sensitive << i_mem_store_fault;
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << i_req_mem_ready;
    sensitive << line_raddr_o;
    sensitive << line_rdata_o;
    sensitive << line_hit_o;
    sensitive << line_rflags_o;
    sensitive << r.req_write;
    sensitive << r.req_cached;
    sensitive << r.req_size;
    sensitive << r.req_prot;
    sensitive << r.req_addr;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.mem_write;
    sensitive << r.mem_addr;
    sensitive << r.cached;
    sensitive << r.rb_resp;
    sensitive << r.write_first;
    sensitive << r.write_flush;
    sensitive << r.mem_wstrb;
    sensitive << r.req_flush;
    sensitive << r.req_flush_addr;
    sensitive << r.req_flush_cnt;
    sensitive << r.flush_cnt;
    sensitive << r.cache_line_i;
    sensitive << r.cache_line_o;
    sensitive << r.init;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

L2CacheLru::~L2CacheLru() {
    delete mem;
}

void L2CacheLru::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, i_nrst.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_type, i_req_type.name());
        sc_trace(o_vcd, i_req_size, i_req_size.name());
        sc_trace(o_vcd, i_req_prot, i_req_prot.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_wdata, i_req_wdata.name());
        sc_trace(o_vcd, i_req_wstrb, i_req_wstrb.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_rdata, o_resp_rdata.name());
        sc_trace(o_vcd, o_resp_status, o_resp_status.name());

        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_cached, o_req_mem_cached.name());
        sc_trace(o_vcd, o_req_mem_size, o_req_mem_size.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_data_ack, i_mem_data_ack.name());
        sc_trace(o_vcd, i_mem_load_fault, i_mem_load_fault.name());
        sc_trace(o_vcd, i_mem_store_fault, i_mem_store_fault.name());

        sc_trace(o_vcd, i_flush_address, i_flush_address.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, line_addr_i, pn + ".linei_addr_i");
        sc_trace(o_vcd, line_wstrb_i, pn + ".linei_wstrb_i");
        sc_trace(o_vcd, line_raddr_o, pn + ".line_raddr_o");
        sc_trace(o_vcd, line_rdata_o, pn + ".line_rdata_o");
        sc_trace(o_vcd, r.rb_resp, pn + ".r_rb_resp");
        sc_trace(o_vcd, r.cached, pn + ".r_cached");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
        sc_trace(o_vcd, r.cache_line_o, pn + ".r_cache_line_o");
        sc_trace(o_vcd, r.write_first, pn + ".r_write_first");
        sc_trace(o_vcd, r.write_flush, pn + ".r_write_flush");
        sc_trace(o_vcd, r.mem_wstrb, pn + ".r_mem_wstrb");
        sc_trace(o_vcd, r.flush_cnt, pn + ".r_flush_cnt");
        sc_trace(o_vcd, r.req_flush, pn + ".r_req_flush");
        sc_trace(o_vcd, r.req_flush_addr, pn + ".r_req_flush_addr");
        sc_trace(o_vcd, r.req_flush_cnt, pn + ".r_req_flush_cnt");
    }
    mem->generateVCD(i_vcd, o_vcd);
}

void L2CacheLru::comb() {
    sc_biguint<L2CACHE_LINE_BITS> vb_cache_line_i_modified;
    sc_biguint<L2CACHE_LINE_BITS> vb_line_rdata_o_modified;
    sc_uint<L2CACHE_BYTES_PER_LINE> vb_line_rdata_o_wstrb;
    
    bool v_req_ready;
    sc_biguint<L2CACHE_LINE_BITS> t_cache_line_i;
    sc_biguint<L1CACHE_LINE_BITS> vb_cached_data;
    sc_biguint<L1CACHE_LINE_BITS> vb_uncached_data;
    bool v_resp_valid;
    sc_biguint<L1CACHE_LINE_BITS> vb_resp_rdata;
    sc_uint<L2_REQ_TYPE_BITS> vb_resp_status;
    bool v_flush;
    bool v_flush_end;
    bool v_line_cs;
    sc_uint<CFG_CPU_ADDR_BITS> vb_line_addr;
    sc_biguint<L2CACHE_LINE_BITS> vb_line_wdata;
    sc_uint<L2CACHE_BYTES_PER_LINE> vb_line_wstrb;
    sc_biguint<L1CACHE_LINE_BITS> vb_req_mask;
    sc_uint<L2TAG_FL_TOTAL> v_line_wflags;
    //sc_uint<CFG_L2_LOG2_BYTES_PER_LINE-CFG_DLOG2_BYTES_PER_LINE> ridx;  // ! zero width??
    int ridx = 0;
    bool v_req_same_line;
    bool v_ready_next;
   
    v = r;

    v_ready_next = 0;
    v_resp_valid = 0;
    vb_resp_rdata = 0;
    vb_resp_status = 0;
    v_req_ready = 0;
    v_flush = 0;
    v_flush_end = 0;
    if (L2CACHE_LINE_BITS == L1CACHE_LINE_BITS) {
        ridx = 0;
    } else {
        ridx = r.req_addr.read()(CFG_L2_LOG2_BYTES_PER_LINE-1,
                                CFG_L2_LOG2_BYTES_PER_LINE-CFG_DLOG2_BYTES_PER_LINE);  // FIXME DLOG2 == L1LOG2
    }

    vb_cached_data = line_rdata_o.read()((ridx+1)*L1CACHE_LINE_BITS - 1,
                                         ridx*L1CACHE_LINE_BITS);
    vb_uncached_data = r.cache_line_i.read()(L1CACHE_LINE_BITS-1, 0);

    v_req_same_line = 0;
    if (r.req_addr.read()(CFG_CPU_ADDR_BITS-1, CFG_L2_LOG2_BYTES_PER_LINE)
        == i_req_addr.read()(CFG_CPU_ADDR_BITS-1, CFG_L2_LOG2_BYTES_PER_LINE)) {
        v_req_same_line = 1;
    }


    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
        if (i_flush_address.read()[0] == 1) {
            v.req_flush_cnt = ~0u;
            v.req_flush_addr = 0;
        } else {
            v.req_flush_cnt = L2CACHE_WAYS-1;
            v.req_flush_addr = i_flush_address.read();
        }
    }

    vb_req_mask = 0;
    for (int i = 0; i < L1CACHE_BYTES_PER_LINE; i++) {
        if (r.req_wstrb.read()[i] == 1) {
            vb_req_mask(8*i+7, 8*i) = ~0u;
        }
    }

    vb_line_rdata_o_modified = line_rdata_o.read();
    vb_cache_line_i_modified = r.cache_line_i.read();
    vb_line_rdata_o_wstrb = 0;
    for (int i = 0; i < L2CACHE_BYTES_PER_LINE/L1CACHE_BYTES_PER_LINE; i++) {
        if (i != ridx) {
            continue;
        }
        vb_line_rdata_o_modified(L1CACHE_LINE_BITS*(i+1)-1, L1CACHE_LINE_BITS*i) =
            (vb_line_rdata_o_modified(L1CACHE_LINE_BITS*(i+1)-1, L1CACHE_LINE_BITS*i)
             & ~vb_req_mask) | (r.req_wdata.read() & vb_req_mask);

        vb_cache_line_i_modified(L1CACHE_LINE_BITS*(i+1)-1, L1CACHE_LINE_BITS*i) =
            (vb_cache_line_i_modified(L1CACHE_LINE_BITS*(i+1)-1, L1CACHE_LINE_BITS*i)
             & ~vb_req_mask) | (r.req_wdata.read() & vb_req_mask);

        vb_line_rdata_o_wstrb(L1CACHE_BYTES_PER_LINE*(i+1)-1, L1CACHE_BYTES_PER_LINE*i) =
            r.req_wstrb.read();
    }
    
    v_line_cs = 0;
    vb_line_addr = r.req_addr.read();
    vb_line_wdata = r.cache_line_i.read();
    vb_line_wstrb = 0;
    v_line_wflags = 0;


    // System Bus access state machine
    switch (r.state.read()) {
    case State_Idle:
        v_ready_next = 1;
        break;
    case State_CheckHit:
        if (line_hit_o.read() == 1) {
            // Hit
            v_resp_valid = 1;
            if (r.req_write.read() == 1) {
                // Modify tagged mem output with request and write back
                v_line_cs = 1;
                v_line_wflags[TAG_FL_VALID] = 1;
                v_line_wflags[L2TAG_FL_DIRTY] = 1;
                v.req_write = 0;
                vb_line_wstrb = vb_line_rdata_o_wstrb;
                vb_line_wdata = vb_line_rdata_o_modified;
                if (v_req_same_line == 1) {
                    // Write address is the same as the next requested, so use it to write
                    // value and update state machine
                    v_ready_next = 1;
                }
                v.state = State_Idle;
            } else {
                vb_resp_rdata = vb_cached_data;
                vb_resp_status = r.rb_resp.read();
                v_ready_next = 1;
                v.state = State_Idle;
            }
        } else {
            // Miss
            v.state = State_CheckMPU;
        }
        break;
    case State_CheckMPU:
        v.req_mem_valid = 1;
        v.mem_write = 0;
        v.state = State_WaitGrant;

        if (r.req_cached.read() == 1) {
            if (line_rflags_o.read()[TAG_FL_VALID] == 1 &&
                line_rflags_o.read()[L2TAG_FL_DIRTY] == 1) {
                v.write_first = 1;
                v.mem_write = 1;
                v.mem_addr = line_raddr_o.read()(CFG_CPU_ADDR_BITS-1,
                            CFG_L2_LOG2_BYTES_PER_LINE) << CFG_L2_LOG2_BYTES_PER_LINE;
            } else {
                v.mem_addr = r.req_addr.read()(CFG_CPU_ADDR_BITS-1,
                            CFG_L2_LOG2_BYTES_PER_LINE) << CFG_L2_LOG2_BYTES_PER_LINE;
            }
            v.mem_wstrb = ~0ul;
            v.cached = 1;
            v.cache_line_o = line_rdata_o;
        } else {
            v.mem_addr = r.req_addr.read();
            v.mem_wstrb = (0, r.req_wstrb.read());
            v.mem_write = r.req_write.read();
            v.cached = 0;
            t_cache_line_i = 0;
            t_cache_line_i(63, 0) = r.req_wdata.read();
            v.cache_line_o = t_cache_line_i;
        }

        v.cache_line_i = 0;
        v.rb_resp = 0;
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            if (r.write_flush.read() == 1 ||
                r.write_first.read() == 1 ||
                (r.req_write.read() == 1 && r.cached.read() == 0)) {
                v.state = State_WriteBus;
            } else {
                // 1. uncached read
                // 2. cached read or write
                v.state = State_WaitResp;
            }
            v.req_mem_valid = 0;
        }
        break;
    case State_WaitResp:
        if (i_mem_data_valid.read()) {
            v.cache_line_i = i_mem_data.read();
            v.state = State_CheckResp;
            if (i_mem_load_fault.read() == 1) {
                v.rb_resp = 0x2;    // SLVERR
            }
        }
        break;
    case State_CheckResp:
        if (r.cached.read() == 0 || r.rb_resp.read() != 0) {
            // uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid = 1;
            vb_resp_rdata = vb_uncached_data;
            vb_resp_status = r.rb_resp.read();
            v.state = State_Idle;
        } else {
            v.state = State_SetupReadAdr;
            v_line_cs = 1;
            v_line_wflags[TAG_FL_VALID] = 1;
            vb_line_wstrb = ~0ul;  // write full line
            if (r.req_write.read() == 1) {
                // Modify tagged mem output with request before write
                v.req_write = 0;
                v_line_wflags[L2TAG_FL_DIRTY] = 1;
                vb_line_wdata = vb_cache_line_i_modified;
                v_resp_valid = 1;
                v.state = State_Idle;
            }
        }
        break;
    case State_SetupReadAdr:
        v.state = State_CheckHit;
        break;
    case State_WriteBus:
        if (i_mem_data_valid.read()) {
            v.state = State_WriteAck;
        }
        break;
    case State_WriteAck:
        if (i_mem_data_ack.read() == 1) {
            if (r.write_flush.read() == 1) {
                // Offloading Cache line on flush request
                v.state = State_FlushAddr;
            } else if (r.write_first.read() == 1) {
                v.mem_addr = r.req_addr.read()(CFG_CPU_ADDR_BITS-1, CFG_L2_LOG2_BYTES_PER_LINE)
                            << CFG_L2_LOG2_BYTES_PER_LINE;
                v.req_mem_valid = 1;
                v.write_first = 0;
                v.mem_write = 0;
                v.state = State_WaitGrant;
            } else {
                // Non-cached write
                v.state = State_Idle;
                v_resp_valid = 1;
                vb_resp_status[1] = i_mem_store_fault.read();  // rb_resp
            }
        }
        break;
    case State_FlushAddr:
        v.state = State_FlushCheck;
        v_flush = 1;
        v.write_flush = 0;
        v.cache_line_i = 0;
        break;
    case State_FlushCheck:
        v.cache_line_o = line_rdata_o.read();
        v_line_wflags = 0;      // flag valid = 0
        vb_line_wstrb = ~0ul;   // write full line
        v_flush = 1;

        if (r.init.read() == 0 &&
            line_rflags_o.read()[TAG_FL_VALID] == 1 &&
            line_rflags_o.read()[L2TAG_FL_DIRTY] == 1) {
            /** Off-load valid line */
            v.write_flush = 1;
            v.mem_addr = line_raddr_o.read();
            v.req_mem_valid = 1;
            v.mem_write = 1;
            v.mem_wstrb = ~0ul;
            v.cached = 1;
            v.state = State_WaitGrant;
        } else {
            /** Write clean line */
            v.state = State_FlushAddr;
            if (r.flush_cnt.read().or_reduce() == 0) {
                v.state = State_Idle;
                v.init = 0;
                v_flush_end = 1;
            }
        }

        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = r.flush_cnt.read() - 1;
            /** Use lsb address bits to manually select memory WAY bank: */
            if (r.req_addr.read()(CFG_L2_LOG2_NWAYS-1, 0) == L2CACHE_WAYS-1) {
                v.req_addr = (r.req_addr.read() + L2CACHE_BYTES_PER_LINE) 
                            & ~((1<<CFG_L2_LOG2_BYTES_PER_LINE)-1);
            } else {
                v.req_addr = r.req_addr.read() + 1;
            }
        }
        break;
    default:;
    }

    if (v_ready_next == 1) {
        if (r.req_flush.read() == 1) {
            v.state = State_FlushAddr;
            v.req_flush = 0;
            v.cache_line_i = 0;
            if (r.req_flush_addr.read()[0] == 1) {
                v.req_addr = 0;
                v.flush_cnt = ~0u;
            } else {
                v.req_addr = r.req_flush_addr.read() & ~((1<<CFG_L2_LOG2_BYTES_PER_LINE)-1);
                v.flush_cnt = r.req_flush_cnt.read();
            }
        } else {
            v_line_cs = i_req_valid.read();
            v_req_ready = 1;
            vb_line_addr = i_req_addr.read();
            if (i_req_valid.read() == 1) {
                v.req_addr = i_req_addr.read();
                v.req_wstrb = i_req_wstrb.read();
                v.req_wdata = i_req_wdata.read();
                switch (i_req_type.read()) {
                case L2_ReqUncachedRead:
                    v.req_write = 0;
                    v.req_cached = 0;
                    break;
                case L2_ReqUncachedWrite:
                    v.req_write = 1;
                    v.req_cached = 0;
                    break;
                case L2_ReqCachedRead:
                    v.req_write = 0;
                    v.req_cached = 1;
                    break;
                case L2_ReqCachedWrite:
                    v.req_write = 1;
                    v.req_cached = 1;
                    break;
                default:
                    printf("!!!!!!!!!!fatal_error\n");
                }
                v.req_size = i_req_size.read();
                v.req_prot = i_req_prot.read();
                v.rb_resp = 0;  // RESP OK
                v.state = State_CheckHit;
            }
        }
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }


    line_cs_i = v_line_cs;
    line_addr_i = vb_line_addr;
    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = v_line_wflags;
    line_flush_i = v_flush;

    o_req_ready = v_req_ready;

    o_req_mem_valid = r.req_mem_valid.read();
    o_req_mem_write = r.mem_write.read();
    o_req_mem_cached = r.cached.read();
    o_req_mem_size = r.req_size.read();
    o_req_mem_prot = r.req_prot.read();
    o_req_mem_addr = r.mem_addr.read();
    o_req_mem_strob = r.mem_wstrb.read();
    o_req_mem_data = r.cache_line_o.read();

    // always 1 clock messages
    o_resp_valid = v_resp_valid;
    o_resp_rdata = vb_resp_rdata;
    o_resp_status = vb_resp_status;
    o_flush_end = v_flush_end;
}

void L2CacheLru::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

