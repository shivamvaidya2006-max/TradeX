import React, { useEffect, useState } from 'react';
import { Layers, RefreshCw } from 'lucide-react';
import { stockApi } from '../services/api';
import { OrderBookData } from '../types';

interface OrderBookProps {
  symbol: string;
  onSelectPrice?: (price: number, type: 'BUY' | 'SELL') => void;
}

export const OrderBook: React.FC<OrderBookProps> = ({ symbol, onSelectPrice }) => {
  const [data, setData] = useState<OrderBookData | null>(null);
  const [loading, setLoading] = useState<boolean>(true);

  const fetchBook = async () => {
    try {
      const res = await stockApi.getOrderBook(symbol);
      if (res.data && res.data.data) {
        setData(res.data.data);
      }
    } catch (e) {
      console.error('Failed to fetch order book', e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchBook();
    const interval = setInterval(fetchBook, 2500);
    return () => clearInterval(interval);
  }, [symbol]);

  if (loading && !data) {
    return (
      <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex items-center justify-center min-h-[220px]">
        <div className="flex items-center space-x-2 text-slate-400 text-xs">
          <RefreshCw className="w-4 h-4 animate-spin text-blue-500" />
          <span>Loading Market Depth...</span>
        </div>
      </div>
    );
  }

  const bids = data?.bids || [];
  const asks = data?.asks || [];

  const maxBidQty = Math.max(...bids.map(b => b.quantity), 1);
  const maxAskQty = Math.max(...asks.map(a => a.quantity), 1);

  const totalBidQty = bids.reduce((acc, b) => acc + b.quantity, 0);
  const totalAskQty = asks.reduce((acc, a) => acc + a.quantity, 0);

  return (
    <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-col justify-between">
      {/* Header */}
      <div className="flex items-center justify-between pb-3 border-b border-slate-800">
        <div className="flex items-center space-x-2">
          <Layers className="w-4 h-4 text-blue-400" />
          <h3 className="text-xs font-bold text-white uppercase tracking-wider">
            Market Depth (5 Best Bids & Offers)
          </h3>
        </div>
        <div className="flex items-center space-x-1.5 text-[10px] text-slate-400 bg-slate-800/80 px-2 py-0.5 rounded-full border border-slate-700/60">
          <span className="w-1.5 h-1.5 rounded-full bg-emerald-400 animate-pulse"></span>
          <span>Live Level 2</span>
        </div>
      </div>

      {/* Tables Row: Bid side vs Ask side */}
      <div className="grid grid-cols-2 gap-4 py-3 text-xs">
        {/* Bid Side */}
        <div>
          <div className="flex items-center justify-between text-[11px] font-semibold text-slate-400 pb-1.5 border-b border-slate-800/60">
            <span className="text-emerald-400 font-bold">BID</span>
            <span>ORDERS</span>
            <span>QTY</span>
            <span className="text-right">PRICE</span>
          </div>
          <div className="space-y-1 mt-1">
            {bids.length === 0 ? (
              <div className="text-[11px] text-slate-500 py-3 text-center">No active buy orders</div>
            ) : (
              bids.map((bid, i) => {
                const depthPct = (bid.quantity / maxBidQty) * 100;
                return (
                  <div
                    key={i}
                    onClick={() => onSelectPrice && onSelectPrice(bid.price, 'SELL')}
                    className="relative flex items-center justify-between py-1 px-1.5 rounded cursor-pointer hover:bg-slate-800/60 transition-colors"
                  >
                    <div
                      className="absolute right-0 top-0 bottom-0 bg-emerald-500/10 rounded-r pointer-events-none"
                      style={{ width: `${depthPct}%` }}
                    />
                    <span className="relative text-[10px] text-slate-500 font-mono">#{i + 1}</span>
                    <span className="relative text-slate-400 font-mono text-[11px]">{bid.orderCount}</span>
                    <span className="relative text-slate-200 font-medium font-mono text-[11px]">{bid.quantity}</span>
                    <span className="relative text-right font-semibold text-emerald-400 font-mono text-[11px]">
                      ₹{bid.price.toFixed(2)}
                    </span>
                  </div>
                );
              })
            )}
          </div>
        </div>

        {/* Ask Side */}
        <div>
          <div className="flex items-center justify-between text-[11px] font-semibold text-slate-400 pb-1.5 border-b border-slate-800/60">
            <span className="text-right">PRICE</span>
            <span>ORDERS</span>
            <span>QTY</span>
            <span className="text-rose-400 font-bold">OFFER</span>
          </div>
          <div className="space-y-1 mt-1">
            {asks.length === 0 ? (
              <div className="text-[11px] text-slate-500 py-3 text-center">No active sell orders</div>
            ) : (
              asks.map((ask, i) => {
                const depthPct = (ask.quantity / maxAskQty) * 100;
                return (
                  <div
                    key={i}
                    onClick={() => onSelectPrice && onSelectPrice(ask.price, 'BUY')}
                    className="relative flex items-center justify-between py-1 px-1.5 rounded cursor-pointer hover:bg-slate-800/60 transition-colors"
                  >
                    <div
                      className="absolute left-0 top-0 bottom-0 bg-rose-500/10 rounded-l pointer-events-none"
                      style={{ width: `${depthPct}%` }}
                    />
                    <span className="relative font-semibold text-rose-400 font-mono text-[11px]">
                      ₹{ask.price.toFixed(2)}
                    </span>
                    <span className="relative text-slate-400 font-mono text-[11px]">{ask.orderCount}</span>
                    <span className="relative text-slate-200 font-medium font-mono text-[11px]">{ask.quantity}</span>
                    <span className="relative text-right text-[10px] text-slate-500 font-mono">#{i + 1}</span>
                  </div>
                );
              })
            )}
          </div>
        </div>
      </div>

      {/* Footer totals & spread */}
      <div className="pt-2 border-t border-slate-800/80 flex items-center justify-between text-[11px] text-slate-400 font-mono">
        <div>
          Total Bids: <span className="text-emerald-400 font-bold">{totalBidQty.toLocaleString()}</span>
        </div>
        <div className="text-slate-500 text-[10px]">
          Spread: ₹{(data?.spread || 0).toFixed(2)}
        </div>
        <div>
          Total Asks: <span className="text-rose-400 font-bold">{totalAskQty.toLocaleString()}</span>
        </div>
      </div>
    </div>
  );
};
