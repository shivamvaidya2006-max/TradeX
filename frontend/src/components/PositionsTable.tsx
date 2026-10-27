import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { ArrowUpRight, ArrowDownRight, Search, XCircle, ExternalLink } from 'lucide-react';
import { Position } from '../types';
import { useMarket } from '../context/MarketContext';

interface PositionsTableProps {
  positions: Position[];
  onRefresh?: () => void;
}

export const PositionsTable: React.FC<PositionsTableProps> = ({ positions, onRefresh }) => {
  const navigate = useNavigate();
  const { stockMap, openOrderModal } = useMarket();
  const [searchTerm, setSearchTerm] = useState('');

  const filteredPositions = positions.filter(
    (p) =>
      p.symbol.toLowerCase().includes(searchTerm.toLowerCase()) ||
      p.companyName.toLowerCase().includes(searchTerm.toLowerCase())
  );

  return (
    <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden flex flex-col">
      {/* Header */}
      <div className="p-4 border-b border-slate-800 flex flex-wrap items-center justify-between gap-3">
        <div className="flex items-center space-x-2">
          <h3 className="text-sm font-bold text-white uppercase tracking-wider">
            Positions ({positions.length})
          </h3>
          <span className="text-xs text-slate-400">Open & Closed Intraday</span>
        </div>

        {/* Filter */}
        <div className="relative">
          <Search className="w-3.5 h-3.5 absolute left-3 top-1/2 -translate-y-1/2 text-slate-400" />
          <input
            type="text"
            placeholder="Filter positions..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            className="pl-8 pr-3 py-1.5 bg-slate-950 border border-slate-800 rounded-lg text-xs text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 w-48 sm:w-60"
          />
        </div>
      </div>

      {/* Table */}
      <div className="overflow-x-auto">
        <table className="w-full text-left text-xs">
          <thead className="bg-slate-950/60 text-slate-400 uppercase text-[10px] tracking-wider border-b border-slate-800/80">
            <tr>
              <th className="py-3 px-4 font-semibold">Product</th>
              <th className="py-3 px-4 font-semibold">Instrument</th>
              <th className="py-3 px-4 font-semibold text-right">Qty</th>
              <th className="py-3 px-4 font-semibold text-right">Avg. Price</th>
              <th className="py-3 px-4 font-semibold text-right">LTP</th>
              <th className="py-3 px-4 font-semibold text-right">Realized P&L</th>
              <th className="py-3 px-4 font-semibold text-right">Unrealized P&L</th>
              <th className="py-3 px-4 font-semibold text-right">Net P&L</th>
              <th className="py-3 px-4 font-semibold text-center">Action</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-800/60">
            {filteredPositions.length === 0 ? (
              <tr>
                <td colSpan={9} className="py-8 text-center text-slate-500">
                  {searchTerm ? 'No positions match your filter' : 'No open trading positions for today.'}
                </td>
              </tr>
            ) : (
              filteredPositions.map((p) => {
                const liveStock = stockMap[p.symbol];
                const ltp = liveStock ? liveStock.currentPrice : p.currentPrice;
                const unrealized = p.quantity !== 0 ? (ltp - p.averagePrice) * p.quantity : 0;
                const totalPnl = p.realizedPnl + unrealized;
                const isPos = totalPnl >= 0;

                return (
                  <tr key={p.id} className="hover:bg-slate-800/40 transition-colors">
                    {/* Product */}
                    <td className="py-3 px-4">
                      <span className="px-2 py-0.5 rounded text-[10px] font-bold bg-slate-800 text-blue-400 border border-slate-700/60">
                        MIS
                      </span>
                    </td>

                    {/* Instrument */}
                    <td className="py-3 px-4">
                      <div
                        onClick={() => navigate(`/stocks/${p.symbol}`)}
                        className="cursor-pointer group flex items-center space-x-1"
                      >
                        <div>
                          <div className="font-bold text-white group-hover:text-blue-400 transition-colors flex items-center gap-1">
                            {p.symbol}
                            <ExternalLink className="w-2.5 h-2.5 opacity-0 group-hover:opacity-100 transition-opacity" />
                          </div>
                          <div className="text-[10px] text-slate-400 truncate max-w-[130px]">
                            {p.companyName}
                          </div>
                        </div>
                      </div>
                    </td>

                    {/* Qty */}
                    <td className="py-3 px-4 text-right font-mono font-medium text-slate-200">
                      {p.quantity}
                    </td>

                    {/* Avg. Price */}
                    <td className="py-3 px-4 text-right font-mono text-slate-300">
                      ₹{p.averagePrice.toFixed(2)}
                    </td>

                    {/* LTP */}
                    <td className="py-3 px-4 text-right font-mono font-semibold text-white">
                      ₹{ltp.toFixed(2)}
                    </td>

                    {/* Realized */}
                    <td className="py-3 px-4 text-right font-mono text-slate-400">
                      <span className={p.realizedPnl >= 0 ? 'text-emerald-400' : 'text-rose-400'}>
                        {p.realizedPnl >= 0 ? '+' : ''}₹{p.realizedPnl.toFixed(2)}
                      </span>
                    </td>

                    {/* Unrealized */}
                    <td className="py-3 px-4 text-right font-mono">
                      <span className={unrealized >= 0 ? 'text-emerald-400' : 'text-rose-400'}>
                        {unrealized >= 0 ? '+' : ''}₹{unrealized.toFixed(2)}
                      </span>
                    </td>

                    {/* Net P&L */}
                    <td className="py-3 px-4 text-right font-mono font-bold">
                      <span className={`inline-flex items-center ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                        {isPos ? <ArrowUpRight className="w-3 h-3 mr-0.5" /> : <ArrowDownRight className="w-3 h-3 mr-0.5" />}
                        {isPos ? '+' : ''}₹{totalPnl.toFixed(2)}
                      </span>
                    </td>

                    {/* Action */}
                    <td className="py-3 px-4 text-center">
                      {p.quantity !== 0 ? (
                        <button
                          type="button"
                          onClick={() => {
                            if (liveStock) openOrderModal(liveStock, p.quantity > 0 ? 'SELL' : 'BUY');
                          }}
                          className="px-2 py-1 bg-rose-500/10 hover:bg-rose-500/20 text-rose-400 rounded text-[11px] font-semibold transition-colors inline-flex items-center gap-1"
                        >
                          <XCircle className="w-3 h-3" />
                          <span>Square Off</span>
                        </button>
                      ) : (
                        <span className="text-[10px] text-slate-500">Closed</span>
                      )}
                    </td>
                  </tr>
                );
              })
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
