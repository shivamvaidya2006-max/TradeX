import React from 'react';
import { NavLink } from 'react-router-dom';
import { 
  LayoutDashboard, LineChart, Bookmark, ShoppingBag, 
  Briefcase, Activity, Wallet, Receipt, 
  ShieldCheck, Settings 
} from 'lucide-react';
import { useAuth } from '../context/AuthContext';

export const Sidebar: React.FC = () => {
  const { isAdmin } = useAuth();

  const navItems = [
    { name: 'Dashboard', path: '/', icon: LayoutDashboard },
    { name: 'Markets', path: '/markets', icon: LineChart },
    { name: 'Watchlists', path: '/watchlists', icon: Bookmark },
    { name: 'Orders', path: '/orders', icon: ShoppingBag },
    { name: 'Holdings', path: '/holdings', icon: Briefcase },
    { name: 'Positions', path: '/positions', icon: Activity },
    { name: 'Funds', path: '/funds', icon: Wallet },
    { name: 'Transactions', path: '/transactions', icon: Receipt },
    { name: 'Settings', path: '/settings', icon: Settings },
  ];

  if (isAdmin) {
    navItems.push({ name: 'Admin Console', path: '/admin', icon: ShieldCheck });
  }

  return (
    <aside className="w-64 bg-slate-900 border-r border-slate-800 flex flex-col flex-shrink-0 h-[calc(100vh-4rem)] sticky top-16 select-none overflow-y-auto">
      <div className="p-4 space-y-1">
        <div className="text-[10px] font-bold text-slate-500 uppercase tracking-wider px-3 mb-2">
          Trading Terminal
        </div>
        {navItems.map((item) => {
          const Icon = item.icon;
          return (
            <NavLink
              key={item.path}
              to={item.path}
              end={item.path === '/'}
              className={({ isActive }) => `
                flex items-center justify-between px-3.5 py-2.5 rounded-xl text-sm font-medium transition-all group
                ${isActive 
                  ? 'bg-blue-600/15 text-blue-400 border border-blue-500/20 shadow-sm' 
                  : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/60'
                }
              `}
            >
              <div className="flex items-center space-x-3">
                <Icon className="w-4 h-4 transition-colors" />
                <span>{item.name}</span>
              </div>
            </NavLink>
          );
        })}
      </div>

      <div className="mt-auto p-4 border-t border-slate-800/80">
        <div className="p-3 bg-slate-800/40 rounded-xl border border-slate-700/40 text-xs">
          <div className="flex items-center space-x-2 text-slate-300 font-semibold mb-1">
            <span className="w-2 h-2 rounded-full bg-emerald-400"></span>
            <span>Paper Trading Engine</span>
          </div>
          <p className="text-[11px] text-slate-400 leading-relaxed">
            Real-time simulated market trading. Zero financial risk.
          </p>
        </div>
      </div>
    </aside>
  );
};
