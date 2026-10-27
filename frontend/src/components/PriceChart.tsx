import React, { useState, useEffect } from 'react';
import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  ReferenceLine,
} from 'recharts';
import { RefreshCw, TrendingUp, TrendingDown, Calendar } from 'lucide-react';
import { stockApi } from '../services/api';
import { PriceHistoryPoint } from '../types';

interface PriceChartProps {
  symbol: string;
  previousClose?: number;
  currentPrice?: number;
}

const RANGES = ['1D', '1W', '1M', '6M', '1Y', '5Y'] as const;
type RangeType = typeof RANGES[number];

export const PriceChart: React.FC<PriceChartProps> = ({
  symbol,
  previousClose,
  currentPrice,
}) => {
  const [range, setRange] = useState<RangeType>('1M');
  const [data, setData] = useState<PriceHistoryPoint[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    let isMounted = true;
    const loadHistory = async () => {
      try {
        setLoading(true);
        const res = await stockApi.getHistory(symbol, range);
        if (isMounted && res.data && res.data.data) {
          setData(res.data.data);
        }
      } catch (e) {
        console.error('Failed to load stock history', e);
      } finally {
        if (isMounted) setLoading(false);
      }
    };

    loadHistory();
    return () => {
      isMounted = false;
    };
  }, [symbol, range]);

  const prices = data.map((d) => d.close);
  const minPrice = prices.length ? Math.min(...prices) * 0.995 : 0;
  const maxPrice = prices.length ? Math.max(...prices) * 1.005 : 100;
  
  const firstPrice = prices.length ? prices[0] : 0;
  const lastPrice = prices.length ? prices[prices.length - 1] : 0;
  const isPositive = lastPrice >= firstPrice;
  const color = isPositive ? '#10b981' : '#f43f5e';

  return (
    <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-col">
      {/* Top Controls Header */}
      <div className="flex flex-wrap items-center justify-between gap-3 mb-4">
        <div className="flex items-center space-x-3">
          <div className="flex items-center space-x-1.5">
            {isPositive ? (
              <TrendingUp className="w-4 h-4 text-emerald-400" />
            ) : (
              <TrendingDown className="w-4 h-4 text-rose-400" />
            )}
            <span className="text-xs font-semibold text-slate-300 uppercase tracking-wider">
              Price Performance
            </span>
          </div>
          {data.length > 0 && (
            <span className="text-xs text-slate-400">
              {((lastPrice - firstPrice) >= 0 ? '+' : '')}
              {(lastPrice - firstPrice).toFixed(2)} (
              {(((lastPrice - firstPrice) / (firstPrice || 1)) * 100).toFixed(2)}%)
            </span>
          )}
        </div>

        {/* Range Selector */}
        <div className="flex items-center space-x-1 bg-slate-950 p-1 rounded-lg border border-slate-800">
          {RANGES.map((r) => (
            <button
              key={r}
              type="button"
              onClick={() => setRange(r)}
              className={`px-2.5 py-1 text-xs font-medium rounded-md transition-all ${
                range === r
                  ? 'bg-blue-600 text-white shadow-sm'
                  : 'text-slate-400 hover:text-slate-200 hover:bg-slate-900'
              }`}
            >
              {r}
            </button>
          ))}
        </div>
      </div>

      {/* Chart Canvas */}
      <div className="h-64 sm:h-72 w-full relative">
        {loading && (
          <div className="absolute inset-0 bg-slate-900/60 backdrop-blur-xs flex items-center justify-center z-10">
            <RefreshCw className="w-6 h-6 animate-spin text-blue-500" />
          </div>
        )}

        {data.length === 0 && !loading ? (
          <div className="h-full flex items-center justify-center text-slate-500 text-sm">
            No historical data available for {symbol}
          </div>
        ) : (
          <ResponsiveContainer width="100%" height="100%">
            <AreaChart data={data} margin={{ top: 10, right: 10, left: -20, bottom: 0 }}>
              <defs>
                <linearGradient id={`gradient-${symbol}`} x1="0" y1="0" x2="0" y2="1">
                  <stop offset="5%" stopColor={color} stopOpacity={0.35} />
                  <stop offset="95%" stopColor={color} stopOpacity={0.0} />
                </linearGradient>
              </defs>

              <XAxis
                dataKey="timestamp"
                stroke="#64748b"
                fontSize={10}
                tickLine={false}
                axisLine={false}
                tickFormatter={(val) => {
                  if (!val) return '';
                  const parts = val.split(' ')[0].split('-');
                  return parts.length >= 3 ? `${parts[1]}/${parts[2]}` : val;
                }}
              />

              <YAxis
                stroke="#64748b"
                fontSize={10}
                domain={[minPrice, maxPrice]}
                tickLine={false}
                axisLine={false}
                tickFormatter={(val) => `₹${val.toFixed(0)}`}
              />

              <Tooltip
                content={({ active, payload }) => {
                  if (active && payload && payload.length) {
                    const d = payload[0].payload as PriceHistoryPoint;
                    return (
                      <div className="bg-slate-950 border border-slate-800 rounded-lg p-2.5 shadow-xl text-xs space-y-1">
                        <div className="text-slate-400 font-medium">{d.timestamp}</div>
                        <div className="text-white font-bold text-sm">
                          Close: ₹{d.close.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
                        </div>
                        <div className="grid grid-cols-2 gap-x-3 text-[11px] text-slate-400 pt-1 border-t border-slate-800/80">
                          <div>Open: ₹{d.open.toFixed(2)}</div>
                          <div>High: ₹{d.high.toFixed(2)}</div>
                          <div>Low: ₹{d.low.toFixed(2)}</div>
                          <div>Vol: {d.volume.toLocaleString()}</div>
                        </div>
                      </div>
                    );
                  }
                  return null;
                }}
              />

              {previousClose && (
                <ReferenceLine
                  y={previousClose}
                  stroke="#94a3b8"
                  strokeDasharray="3 3"
                  label={{
                    value: `Prev Close ₹${previousClose.toFixed(2)}`,
                    fill: '#94a3b8',
                    fontSize: 9,
                    position: 'insideBottomRight',
                  }}
                />
              )}

              <Area
                type="monotone"
                dataKey="close"
                stroke={color}
                strokeWidth={2}
                fillOpacity={1}
                fill={`url(#gradient-${symbol})`}
              />
            </AreaChart>
          </ResponsiveContainer>
        )}
      </div>
    </div>
  );
};
