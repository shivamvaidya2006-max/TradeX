import React from 'react';
import { User, ShieldCheck, Cpu, Terminal, BookOpen, AlertCircle, Info, ExternalLink } from 'lucide-react';
import { useAuth } from '../context/AuthContext';

export const Settings: React.FC = () => {
  const { user } = useAuth();

  return (
    <div className="space-y-6 max-w-4xl">
      <div>
        <h1 className="text-2xl font-black text-white tracking-tight">
          Settings & System Overview
        </h1>
        <p className="text-xs sm:text-sm text-slate-400 mt-1">
          Profile details, architectural specifications, and educational compliance
        </p>
      </div>

      {/* User Profile Card */}
      <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 space-y-4">
        <div className="flex items-center space-x-3 pb-3 border-b border-slate-800">
          <div className="w-10 h-10 rounded-full bg-blue-600/20 border border-blue-500/30 text-blue-400 flex items-center justify-center font-bold">
            {user?.fullName?.charAt(0) || user?.username?.charAt(0) || 'U'}
          </div>
          <div>
            <h2 className="text-base font-bold text-white">{user?.fullName || 'Trader'}</h2>
            <p className="text-xs text-slate-400">@{user?.username} • Role: {user?.role}</p>
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 text-xs font-mono">
          <div className="bg-slate-950 p-3 rounded-xl border border-slate-800">
            <span className="text-slate-500 block text-[10px] uppercase font-sans">Email</span>
            <span className="text-slate-200">{user?.email || 'trader@tradex.local'}</span>
          </div>
          <div className="bg-slate-950 p-3 rounded-xl border border-slate-800">
            <span className="text-slate-500 block text-[10px] uppercase font-sans">Account ID</span>
            <span className="text-slate-200">USR-{user?.id?.toString().padStart(5, '0') || '00001'}</span>
          </div>
        </div>
      </div>

      {/* Architecture & Tech Specs Card */}
      <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 space-y-4">
        <div className="flex items-center space-x-2 pb-3 border-b border-slate-800">
          <Cpu className="w-5 h-5 text-blue-400" />
          <h3 className="text-sm font-bold text-white uppercase tracking-wider">
            Technical Architecture Specifications
          </h3>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 text-xs">
          <div className="bg-slate-950 p-4 rounded-xl border border-slate-800 space-y-1">
            <div className="text-blue-400 font-bold">Backend Engine</div>
            <div className="text-slate-300">Modern C++17 / Clang++ 22.1</div>
            <div className="text-[11px] text-slate-500">Winsock2 TCP Server with Content-Length loop & RFC-6455 WebSockets</div>
          </div>

          <div className="bg-slate-950 p-4 rounded-xl border border-slate-800 space-y-1">
            <div className="text-emerald-400 font-bold">Relational Database</div>
            <div className="text-slate-300">MySQL 8.0 Enterprise</div>
            <div className="text-[11px] text-slate-500">12 Normalized tables, B-Tree indexes, foreign keys, transaction logs</div>
          </div>

          <div className="bg-slate-950 p-4 rounded-xl border border-slate-800 space-y-1">
            <div className="text-purple-400 font-bold">Matching Engine</div>
            <div className="text-slate-300">Continuous Double Auction</div>
            <div className="text-[11px] text-slate-500">Price-Time priority double-auction matching engine with sub-millisecond execution</div>
          </div>

          <div className="bg-slate-950 p-4 rounded-xl border border-slate-800 space-y-1">
            <div className="text-amber-400 font-bold">Frontend Stack</div>
            <div className="text-slate-300">React 19 + TypeScript + Vite + Tailwind CSS</div>
            <div className="text-[11px] text-slate-500">Recharts financial graphs, Lucide icons, responsive layout</div>
          </div>
        </div>
      </div>

      {/* Educational Disclaimer Banner */}
      <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 space-y-3">
        <div className="flex items-center space-x-2 text-amber-400">
          <AlertCircle className="w-5 h-5 shrink-0" />
          <h3 className="text-sm font-bold uppercase tracking-wider">
            Important Educational Disclaimer
          </h3>
        </div>
        <p className="text-xs text-slate-300 leading-relaxed">
          TradeX is exclusively designed and built as an educational paper-trading and stock-market simulation system for a BTech computer science and engineering capstone project. 
          TradeX does <strong>NOT</strong> execute real financial orders, does <strong>NOT</strong> accept real currency, does <strong>NOT</strong> connect to real SEBI/NSE/BSE broker accounts, and cannot incur any actual financial liability or loss. All prices, order book depths, portfolios, and ledger transactions are generated internally via deterministic and stochastic simulations.
        </p>
      </div>
    </div>
  );
};
