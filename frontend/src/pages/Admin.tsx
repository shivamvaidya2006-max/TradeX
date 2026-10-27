import React, { useState, useEffect } from 'react';
import {
  ShieldAlert,
  Play,
  Square,
  Users,
  Server,
  Activity,
  CheckCircle2,
  AlertCircle,
  RefreshCw,
} from 'lucide-react';
import { adminApi, stockApi } from '../services/api';
import { useAuth } from '../context/AuthContext';
import { useMarket } from '../context/MarketContext';

export const Admin: React.FC = () => {
  const { user } = useAuth();
  const { marketOpen, refreshMarketData } = useMarket();

  const [users, setUsers] = useState<any[]>([]);
  const [systemStats, setSystemStats] = useState<any>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [actionMessage, setActionMessage] = useState<string | null>(null);

  const fetchAdminData = async () => {
    try {
      setLoading(true);
      const [uRes, sRes] = await Promise.all([
        adminApi.getUsers(),
        adminApi.getSystemStats(),
      ]);

      if (uRes.data?.data) {
        setUsers(uRes.data.data);
      }
      if (sRes.data?.data) {
        setSystemStats(sRes.data.data);
      }
    } catch (e) {
      console.error('Failed to load admin data', e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchAdminData();
  }, []);

  const handleStartMarket = async () => {
    try {
      await adminApi.startMarket();
      setActionMessage('Market simulation started successfully!');
      refreshMarketData();
      fetchAdminData();
      setTimeout(() => setActionMessage(null), 3000);
    } catch (e) {
      console.error(e);
    }
  };

  const handleStopMarket = async () => {
    try {
      await adminApi.stopMarket();
      setActionMessage('Market simulation halted.');
      refreshMarketData();
      fetchAdminData();
      setTimeout(() => setActionMessage(null), 3000);
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex flex-wrap items-center justify-between gap-4">
        <div>
          <div className="flex items-center space-x-2">
            <ShieldAlert className="w-6 h-6 text-amber-400" />
            <h1 className="text-2xl font-black text-white tracking-tight">
              TradeX System Administration & Simulation Controls
            </h1>
          </div>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Engineered for administrators: manage user accounts, market simulators, and C++ memory state
          </p>
        </div>

        {actionMessage && (
          <div className="px-3.5 py-1.5 bg-emerald-500/10 border border-emerald-500/20 text-emerald-400 rounded-xl text-xs font-semibold">
            {actionMessage}
          </div>
        )}
      </div>

      {/* Simulator Control Card */}
      <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 flex flex-wrap items-center justify-between gap-6">
        <div>
          <div className="flex items-center space-x-3">
            <div className={`w-3 h-3 rounded-full ${marketOpen ? 'bg-emerald-500 animate-pulse' : 'bg-rose-500'}`} />
            <h2 className="text-lg font-bold text-white">
              C++ Brownian Motion Price Generator Status: {marketOpen ? 'ACTIVE' : 'HALTED'}
            </h2>
          </div>
          <p className="text-xs text-slate-400 mt-1">
            Simulates realistic stochastic price volatility, volume fluctuations, and high-frequency order book updates
          </p>
        </div>

        <div className="flex items-center space-x-3">
          <button
            onClick={handleStartMarket}
            disabled={marketOpen}
            className="px-4 py-2 bg-emerald-600 hover:bg-emerald-500 text-white rounded-xl text-xs font-bold shadow-md transition-all flex items-center space-x-1.5 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            <Play className="w-3.5 h-3.5" />
            <span>Resume Simulation</span>
          </button>
          <button
            onClick={handleStopMarket}
            disabled={!marketOpen}
            className="px-4 py-2 bg-rose-600 hover:bg-rose-500 text-white rounded-xl text-xs font-bold shadow-md transition-all flex items-center space-x-1.5 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            <Square className="w-3.5 h-3.5" />
            <span>Pause Simulation</span>
          </button>
        </div>
      </div>

      {/* System Metrics */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5">
          <div className="flex justify-between items-center text-slate-400 text-xs font-semibold uppercase">
            <span>Server Architecture</span>
            <Server className="w-4 h-4 text-blue-400" />
          </div>
          <div className="text-lg font-bold text-white font-mono mt-2">
            C++17 Winsock2 REST
          </div>
          <div className="text-[11px] text-slate-500 mt-0.5">High Performance Daemon</div>
        </div>

        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5">
          <div className="flex justify-between items-center text-slate-400 text-xs font-semibold uppercase">
            <span>Total Registered Users</span>
            <Users className="w-4 h-4 text-emerald-400" />
          </div>
          <div className="text-2xl font-black text-white font-mono mt-2">
            {users.length}
          </div>
          <div className="text-[11px] text-slate-500 mt-0.5">Paper-Trading Accounts</div>
        </div>

        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5">
          <div className="flex justify-between items-center text-slate-400 text-xs font-semibold uppercase">
            <span>Database Backend</span>
            <Activity className="w-4 h-4 text-amber-400" />
          </div>
          <div className="text-lg font-bold text-white font-mono mt-2">
            MySQL 8.0 (Native)
          </div>
          <div className="text-[11px] text-slate-500 mt-0.5">Connection Pool Active</div>
        </div>

        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5">
          <div className="flex justify-between items-center text-slate-400 text-xs font-semibold uppercase">
            <span>Execution Engines</span>
            <ShieldAlert className="w-4 h-4 text-purple-400" />
          </div>
          <div className="text-lg font-bold text-white font-mono mt-2">
            Active
          </div>
          <div className="text-[11px] text-slate-500 mt-0.5">Matching & Risk Controls</div>
        </div>
      </div>

      {/* User Directory Table */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden">
        <div className="p-4 border-b border-slate-800 flex items-center justify-between">
          <h3 className="text-xs font-bold text-white uppercase tracking-wider">
            Registered System Users ({users.length})
          </h3>
          <button
            onClick={fetchAdminData}
            className="text-xs text-blue-400 hover:text-blue-300 font-medium"
          >
            Refresh
          </button>
        </div>

        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs">
            <thead className="bg-slate-950/70 text-slate-400 uppercase text-[10px] tracking-wider border-b border-slate-800/80">
              <tr>
                <th className="py-3 px-4 font-semibold">User ID</th>
                <th className="py-3 px-4 font-semibold">Full Name</th>
                <th className="py-3 px-4 font-semibold">Username</th>
                <th className="py-3 px-4 font-semibold">Email</th>
                <th className="py-3 px-4 font-semibold">Role</th>
                <th className="py-3 px-4 font-semibold">Status</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-800/60 font-mono">
              {users.map((u) => (
                <tr key={u.id} className="hover:bg-slate-800/40">
                  <td className="py-3 px-4 text-slate-400">USR-{u.id}</td>
                  <td className="py-3 px-4 text-white font-sans font-bold">{u.fullName}</td>
                  <td className="py-3 px-4 text-slate-300">@{u.username}</td>
                  <td className="py-3 px-4 text-slate-400 font-sans">{u.email}</td>
                  <td className="py-3 px-4">
                    <span className={`px-2 py-0.5 rounded text-[10px] font-bold ${
                      u.role === 'ADMIN'
                        ? 'bg-purple-500/10 text-purple-400 border border-purple-500/30'
                        : 'bg-blue-500/10 text-blue-400 border border-blue-500/30'
                    }`}>
                      {u.role}
                    </span>
                  </td>
                  <td className="py-3 px-4">
                    <span className="text-emerald-400 text-[11px] font-bold">
                      ACTIVE
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};
