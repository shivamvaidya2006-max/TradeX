import React from 'react';
import { ArrowUpRight, ArrowDownRight } from 'lucide-react';
import { useMarket } from '../context/MarketContext';

export const MarketTicker: React.FC = () => {
  const { marketOverview, stocks, priceFlashes } = useMarket();

  if (!marketOverview) return null;

  const indices = [
    { name: 'NIFTY 50', data: marketOverview.nifty50 },
    { name: 'BANK NIFTY', data: marketOverview.bankNifty },
    { name: 'SENSEX', data: marketOverview.sensex },
  ];

  return (
    <div className="bg-slate-900 border-b border-slate-800 text-xs px-4 py-2 overflow-x-auto flex items-center space-x-6 select-none scrollbar-none">
      {/* Indices */}
      <div className="flex items-center space-x-6 flex-shrink-0">
        {indices.map((idx) => {
          const isPos = idx.data.change >= 0;
          return (
            <div key={idx.name} className="flex items-center space-x-2">
              <span className="font-semibold text-slate-300">{idx.name}</span>
              <span className="font-mono font-bold text-slate-100">{idx.data.price.toFixed(2)}</span>
              <span className={`flex items-center font-mono text-[11px] ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                {isPos ? <ArrowUpRight className="w-3 h-3 mr-0.5" /> : <ArrowDownRight className="w-3 h-3 mr-0.5" />}
                {isPos ? '+' : ''}{idx.data.change.toFixed(2)} ({idx.data.changePercent.toFixed(2)}%)
              </span>
            </div>
          );
        })}
      </div>

      <div className="h-4 w-[1px] bg-slate-800 flex-shrink-0"></div>

      {/* Featured Live Stocks with flashing animations */}
      <div className="flex items-center space-x-6 flex-shrink-0">
        {stocks.slice(0, 8).map((stock) => {
          const isPos = stock.change >= 0;
          const flash = priceFlashes[stock.symbol];
          return (
            <div
              key={stock.symbol}
              className={`flex items-center space-x-2 px-2 py-0.5 rounded transition-all duration-300 ${
                flash === 'up' ? 'flash-green' : flash === 'down' ? 'flash-red' : ''
              }`}
            >
              <span className="font-semibold text-slate-400">{stock.symbol}</span>
              <span className="font-mono font-medium text-slate-200">₹{stock.currentPrice.toFixed(2)}</span>
              <span className={`text-[11px] font-mono ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                {isPos ? '+' : ''}{stock.changePercent.toFixed(2)}%
              </span>
            </div>
          );
        })}
      </div>
    </div>
  );
};
