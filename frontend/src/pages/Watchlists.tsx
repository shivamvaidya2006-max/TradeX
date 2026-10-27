import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import {
  Bookmark,
  Plus,
  Trash2,
  ExternalLink,
  ArrowUpRight,
  ArrowDownRight,
  Search,
  Check,
} from 'lucide-react';
import { useMarket } from '../context/MarketContext';
import { watchlistApi, stockApi } from '../services/api';
import { Watchlist, Stock } from '../types';

export const Watchlists: React.FC = () => {
  const navigate = useNavigate();
  const { stocks, stockMap, priceFlashes, openOrderModal } = useMarket();

  const [watchlists, setWatchlists] = useState<Watchlist[]>([]);
  const [activeWatchlistId, setActiveWatchlistId] = useState<number>(1);
  const [newWatchlistName, setNewWatchlistName] = useState('');
  const [showAddStockModal, setShowAddStockModal] = useState(false);
  const [stockSearchQuery, setStockSearchQuery] = useState('');
  const [loading, setLoading] = useState<boolean>(true);

  const loadWatchlists = async () => {
    try {
      setLoading(true);
      const res = await watchlistApi.getAll();
      if (res.data && res.data.data) {
        setWatchlists(res.data.data);
        if (res.data.data.length > 0 && !activeWatchlistId) {
          setActiveWatchlistId(res.data.data[0].id);
        }
      }
    } catch (e) {
      console.error('Failed to load watchlists', e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadWatchlists();
  }, []);

  const activeWatchlist = watchlists.find((w) => w.id === activeWatchlistId) || watchlists[0];

  const handleCreateWatchlist = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!newWatchlistName.trim()) return;
    try {
      const res = await watchlistApi.create(newWatchlistName.trim());
      setNewWatchlistName('');
      loadWatchlists();
    } catch (e) {
      console.error('Failed to create watchlist', e);
    }
  };

  const handleAddStock = async (stockId: number) => {
    if (!activeWatchlist) return;
    try {
      await watchlistApi.addItem(activeWatchlist.id, stockId);
      setShowAddStockModal(false);
      setStockSearchQuery('');
      loadWatchlists();
    } catch (e) {
      console.error('Failed to add stock to watchlist', e);
    }
  };

  const handleRemoveStock = async (stockId: number) => {
    if (!activeWatchlist) return;
    try {
      await watchlistApi.removeItem(activeWatchlist.id, stockId);
      loadWatchlists();
    } catch (e) {
      console.error('Failed to remove stock from watchlist', e);
    }
  };

  const availableStocksToAdd = stocks.filter((s) => {
    const isAlreadyIn = activeWatchlist?.items?.some((item) => item.id === s.id);
    const matches =
      s.symbol.toLowerCase().includes(stockSearchQuery.toLowerCase()) ||
      s.companyName.toLowerCase().includes(stockSearchQuery.toLowerCase());
    return !isAlreadyIn && matches;
  });

  return (
    <div className="space-y-6">
      {/* Title & Add Watchlist Bar */}
      <div className="flex flex-wrap items-center justify-between gap-4">
        <div>
          <h1 className="text-2xl font-black text-white tracking-tight">
            Watchlists
          </h1>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Track real-time prices and order books for your favorite NSE / BSE equities
          </p>
        </div>

        <form onSubmit={handleCreateWatchlist} className="flex items-center space-x-2">
          <input
            type="text"
            placeholder="New watchlist name..."
            value={newWatchlistName}
            onChange={(e) => setNewWatchlistName(e.target.value)}
            className="px-3 py-1.5 bg-slate-900 border border-slate-800 rounded-lg text-xs text-white placeholder-slate-500 focus:outline-none focus:border-blue-500"
          />
          <button
            type="submit"
            className="px-3 py-1.5 bg-blue-600 hover:bg-blue-500 text-white rounded-lg text-xs font-semibold flex items-center space-x-1"
          >
            <Plus className="w-3.5 h-3.5" />
            <span>Create</span>
          </button>
        </form>
      </div>

      {/* Tabs */}
      <div className="flex items-center justify-between border-b border-slate-800 pb-2">
        <div className="flex items-center space-x-2 overflow-x-auto">
          {watchlists.map((w) => (
            <button
              key={w.id}
              onClick={() => setActiveWatchlistId(w.id)}
              className={`px-4 py-2 rounded-xl text-xs font-bold transition-all whitespace-nowrap flex items-center space-x-2 ${
                activeWatchlist?.id === w.id
                  ? 'bg-blue-600 text-white shadow-md shadow-blue-600/30'
                  : 'bg-slate-900 text-slate-400 hover:text-white border border-slate-800'
              }`}
            >
              <Bookmark className="w-3.5 h-3.5" />
              <span>{w.name}</span>
              <span className="text-[10px] px-1.5 py-0.2 bg-black/30 rounded-full font-mono">
                {w.items?.length || 0}
              </span>
            </button>
          ))}
        </div>

        {activeWatchlist && (
          <button
            onClick={() => setShowAddStockModal(true)}
            className="px-3.5 py-1.5 bg-slate-900 hover:bg-slate-800 border border-slate-700 text-blue-400 rounded-xl text-xs font-semibold flex items-center space-x-1.5 transition-colors shrink-0"
          >
            <Plus className="w-3.5 h-3.5" />
            <span>Add Stock</span>
          </button>
        )}
      </div>

      {/* Watchlist Stock Table */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden">
        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs">
            <thead className="bg-slate-950/70 text-slate-400 uppercase text-[10px] tracking-wider border-b border-slate-800/80">
              <tr>
                <th className="py-3 px-4 font-semibold">Instrument</th>
                <th className="py-3 px-4 font-semibold">Sector</th>
                <th className="py-3 px-4 font-semibold text-right">LTP (₹)</th>
                <th className="py-3 px-4 font-semibold text-right">Change (₹)</th>
                <th className="py-3 px-4 font-semibold text-right">Change (%)</th>
                <th className="py-3 px-4 font-semibold text-right">Day High / Low</th>
                <th className="py-3 px-4 font-semibold text-right">Volume</th>
                <th className="py-3 px-4 font-semibold text-center">Trade</th>
                <th className="py-3 px-4 font-semibold text-center">Action</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-800/60">
              {!activeWatchlist || !activeWatchlist.items || activeWatchlist.items.length === 0 ? (
                <tr>
                  <td colSpan={9} className="py-12 text-center text-slate-500">
                    No stocks in this watchlist. Click "+ Add Stock" to start tracking.
                  </td>
                </tr>
              ) : (
                activeWatchlist.items.map((item) => {
                  const liveStock = stockMap[item.symbol] || item;
                  const isPos = liveStock.change >= 0;
                  const flash = priceFlashes[liveStock.symbol];
                  const flashClass = flash === 'up' ? 'flash-green' : flash === 'down' ? 'flash-red' : '';

                  return (
                    <tr
                      key={item.id}
                      className={`hover:bg-slate-800/40 transition-colors ${flashClass}`}
                    >
                      {/* Instrument */}
                      <td className="py-3 px-4">
                        <div
                          onClick={() => navigate(`/stocks/${liveStock.symbol}`)}
                          className="cursor-pointer group flex items-center space-x-2"
                        >
                          <div>
                            <div className="font-bold text-white group-hover:text-blue-400 transition-colors flex items-center gap-1.5">
                              {liveStock.symbol}
                              <span className="text-[10px] text-slate-400 font-normal">
                                {liveStock.exchange}
                              </span>
                              <ExternalLink className="w-2.5 h-2.5 opacity-0 group-hover:opacity-100 transition-opacity text-blue-400" />
                            </div>
                            <div className="text-[11px] text-slate-400 truncate max-w-[150px]">
                              {liveStock.companyName}
                            </div>
                          </div>
                        </div>
                      </td>

                      {/* Sector */}
                      <td className="py-3 px-4">
                        <span className="px-2 py-0.5 rounded text-[10px] font-medium bg-slate-800 text-slate-300">
                          {liveStock.sector}
                        </span>
                      </td>

                      {/* LTP */}
                      <td className="py-3 px-4 text-right font-mono font-bold text-white text-sm">
                        ₹{liveStock.currentPrice.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
                      </td>

                      {/* Change */}
                      <td className="py-3 px-4 text-right font-mono font-semibold">
                        <span className={isPos ? 'text-emerald-400' : 'text-rose-400'}>
                          {isPos ? '+' : ''}{liveStock.change.toFixed(2)}
                        </span>
                      </td>

                      {/* Change % */}
                      <td className="py-3 px-4 text-right font-mono font-semibold">
                        <span className={`inline-flex items-center ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                          {isPos ? <ArrowUpRight className="w-2.5 h-2.5 mr-0.5" /> : <ArrowDownRight className="w-2.5 h-2.5 mr-0.5" />}
                          {isPos ? '+' : ''}{liveStock.changePercent.toFixed(2)}%
                        </span>
                      </td>

                      {/* High/Low */}
                      <td className="py-3 px-4 text-right font-mono text-[11px] text-slate-400">
                        <div>H: ₹{liveStock.dayHigh.toFixed(2)}</div>
                        <div>L: ₹{liveStock.dayLow.toFixed(2)}</div>
                      </td>

                      {/* Volume */}
                      <td className="py-3 px-4 text-right font-mono text-slate-300">
                        {liveStock.volume.toLocaleString('en-IN')}
                      </td>

                      {/* Trade Buttons */}
                      <td className="py-3 px-4 text-center">
                        <div className="flex items-center justify-center space-x-1.5">
                          <button
                            type="button"
                            onClick={() => openOrderModal(liveStock, 'BUY')}
                            className="px-2.5 py-1 bg-blue-600 hover:bg-blue-500 text-white rounded text-xs font-bold shadow"
                          >
                            B
                          </button>
                          <button
                            type="button"
                            onClick={() => openOrderModal(liveStock, 'SELL')}
                            className="px-2.5 py-1 bg-rose-600 hover:bg-rose-500 text-white rounded text-xs font-bold shadow"
                          >
                            S
                          </button>
                        </div>
                      </td>

                      {/* Delete Action */}
                      <td className="py-3 px-4 text-center">
                        <button
                          type="button"
                          onClick={() => handleRemoveStock(item.id)}
                          title="Remove from watchlist"
                          className="p-1.5 text-slate-500 hover:text-rose-400 hover:bg-rose-500/10 rounded-lg transition-colors"
                        >
                          <Trash2 className="w-3.5 h-3.5" />
                        </button>
                      </td>
                    </tr>
                  );
                })
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Add Stock Modal */}
      {showAddStockModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-950/80 backdrop-blur-xs">
          <div className="bg-slate-900 border border-slate-800 rounded-2xl w-full max-w-md p-5 space-y-4 shadow-2xl">
            <div className="flex items-center justify-between pb-3 border-b border-slate-800">
              <h3 className="text-sm font-bold text-white">
                Add Stock to {activeWatchlist?.name}
              </h3>
              <button
                onClick={() => setShowAddStockModal(false)}
                className="text-slate-400 hover:text-white text-xs font-bold"
              >
                ✕
              </button>
            </div>

            <div className="relative">
              <Search className="w-4 h-4 absolute left-3 top-1/2 -translate-y-1/2 text-slate-400" />
              <input
                type="text"
                placeholder="Search stocks to add..."
                value={stockSearchQuery}
                onChange={(e) => setStockSearchQuery(e.target.value)}
                autoFocus
                className="w-full pl-9 pr-3 py-2 bg-slate-950 border border-slate-800 rounded-xl text-xs text-white placeholder-slate-500 focus:outline-none focus:border-blue-500"
              />
            </div>

            <div className="max-h-60 overflow-y-auto divide-y divide-slate-800/60">
              {availableStocksToAdd.slice(0, 10).map((stk) => (
                <div
                  key={stk.id}
                  className="py-2.5 px-3 flex items-center justify-between hover:bg-slate-800/50 rounded-lg cursor-pointer"
                  onClick={() => handleAddStock(stk.id)}
                >
                  <div>
                    <div className="font-bold text-xs text-white">{stk.symbol}</div>
                    <div className="text-[10px] text-slate-400">{stk.companyName}</div>
                  </div>
                  <div className="flex items-center space-x-2">
                    <span className="font-mono text-xs font-bold text-slate-200">
                      ₹{stk.currentPrice.toFixed(2)}
                    </span>
                    <button className="p-1 bg-blue-600/20 hover:bg-blue-600/40 text-blue-400 rounded">
                      <Plus className="w-3.5 h-3.5" />
                    </button>
                  </div>
                </div>
              ))}
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
