import React, { useState, useEffect, useRef } from 'react';
import { useNavigate, Link } from 'react-router-dom';
import { 
  Search, Bell, User as UserIcon, LogOut, ShieldAlert, 
  TrendingUp, Radio, ChevronDown, CheckCircle, Clock 
} from 'lucide-react';
import { useAuth } from '../context/AuthContext';
import { useMarket } from '../context/MarketContext';
import { stockApi, notificationApi } from '../services/api';
import { NotificationItem } from '../types';

interface NavbarProps {
  onSelectStock?: (symbol: string) => void;
}

export const Navbar: React.FC<NavbarProps> = ({ onSelectStock }) => {
  const navigate = useNavigate();
  const { user, logout, isAdmin } = useAuth();
  const { marketOpen } = useMarket();

  // Search state
  const [searchQuery, setSearchQuery] = useState('');
  const [suggestions, setSuggestions] = useState<any[]>([]);
  const [showDropdown, setShowDropdown] = useState(false);
  const searchRef = useRef<HTMLDivElement>(null);

  // Notification state
  const [notifications, setNotifications] = useState<NotificationItem[]>([]);
  const [showNotifications, setShowNotifications] = useState(false);
  const notifRef = useRef<HTMLDivElement>(null);

  // Profile menu state
  const [showProfile, setShowProfile] = useState(false);
  const profileRef = useRef<HTMLDivElement>(null);

  // Close popups on click outside
  useEffect(() => {
    const handleClickOutside = (e: MouseEvent) => {
      if (searchRef.current && !searchRef.current.contains(e.target as Node)) {
        setShowDropdown(false);
      }
      if (notifRef.current && !notifRef.current.contains(e.target as Node)) {
        setShowNotifications(false);
      }
      if (profileRef.current && !profileRef.current.contains(e.target as Node)) {
        setShowProfile(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  // Fetch Trie autocomplete suggestions as user types
  useEffect(() => {
    if (!searchQuery.trim()) {
      setSuggestions([]);
      setShowDropdown(false);
      return;
    }

    const timer = setTimeout(async () => {
      try {
        const res = await stockApi.search(searchQuery.trim());
        if (res.data && res.data.data) {
          setSuggestions(res.data.data);
          setShowDropdown(true);
        }
      } catch (err) {
        console.error('Trie search error', err);
      }
    }, 150);

    return () => clearTimeout(timer);
  }, [searchQuery]);

  // Fetch notifications
  useEffect(() => {
    const fetchNotifs = async () => {
      try {
        const res = await notificationApi.getAll();
        if (res.data && res.data.data) {
          setNotifications(res.data.data);
        }
      } catch {}
    };
    fetchNotifs();
  }, []);

  const handleSelectSuggestion = (symbol: string) => {
    setSearchQuery('');
    setShowDropdown(false);
    if (onSelectStock) {
      onSelectStock(symbol);
    } else {
      navigate(`/markets/${symbol}`);
    }
  };

  const unreadCount = notifications.filter(n => !n.isRead).length;

  return (
    <header className="sticky top-0 z-40 bg-slate-900/95 backdrop-blur-md border-b border-slate-800 text-slate-200">
      <div className="flex items-center justify-between px-4 lg:px-6 h-16">
        {/* Brand Identity */}
        <div className="flex items-center space-x-6">
          <Link to="/" className="flex items-center space-x-3 group">
            <div className="w-10 h-10 rounded-xl bg-gradient-to-tr from-blue-600 via-indigo-600 to-cyan-400 flex items-center justify-center shadow-lg shadow-blue-500/20 group-hover:scale-105 transition-transform">
              <TrendingUp className="w-6 h-6 text-white" />
            </div>
            <div>
              <div className="flex items-center space-x-2">
                <span className="text-xl font-bold tracking-tight bg-gradient-to-r from-white via-slate-200 to-slate-400 bg-clip-text text-transparent">
                  Trade<span className="text-blue-500">X</span>
                </span>
                <span className="text-[10px] font-semibold tracking-wider uppercase px-1.5 py-0.5 rounded bg-blue-500/10 text-blue-400 border border-blue-500/20">
                  Paper
                </span>
              </div>
              <p className="text-[11px] text-slate-400 hidden sm:block">Smart Trading. Simulated. Data Driven.</p>
            </div>
          </Link>

          {/* Market Status Live Badge */}
          <div className="hidden md:flex items-center space-x-2 px-3 py-1 rounded-full bg-slate-800/80 border border-slate-700/60 text-xs">
            <span className="relative flex h-2 w-2">
              <span className={`animate-ping absolute inline-flex h-full w-full rounded-full opacity-75 ${marketOpen ? 'bg-emerald-400' : 'bg-rose-400'}`}></span>
              <span className={`relative inline-flex rounded-full h-2 w-2 ${marketOpen ? 'bg-emerald-500' : 'bg-rose-500'}`}></span>
            </span>
            <span className="font-medium text-slate-300">
              {marketOpen ? 'MARKET OPEN (NSE)' : 'MARKET CLOSED'}
            </span>
            <span className="text-slate-500 font-mono text-[10px]">09:15 - 15:30</span>
          </div>
        </div>

        {/* Global Search Bar with Trie Autocomplete */}
        <div ref={searchRef} className="relative flex-1 max-w-md mx-4 lg:mx-8">
          <div className="relative">
            <Search className="absolute left-3.5 top-1/2 -translate-y-1/2 w-4 h-4 text-slate-400" />
            <input
              type="text"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              onFocus={() => searchQuery.trim() && setShowDropdown(true)}
              placeholder="Search eg: TCS, RELIANCE, INFY, HDFC..."
              className="w-full pl-10 pr-4 py-2 bg-slate-800/90 border border-slate-700/80 rounded-xl text-sm text-slate-100 placeholder-slate-400 focus:outline-none focus:ring-2 focus:ring-blue-500/50 focus:border-blue-500 transition-all shadow-inner"
            />
          </div>

          {/* Trie Autocomplete Dropdown */}
          {showDropdown && suggestions.length > 0 && (
            <div className="absolute left-0 right-0 mt-2 bg-slate-900 border border-slate-700 rounded-xl shadow-2xl overflow-hidden z-50 animate-in fade-in slide-in-from-top-2 duration-150">
              <div className="px-3 py-1.5 bg-slate-800/60 border-b border-slate-800 text-[10px] font-semibold text-slate-400 uppercase tracking-wider flex justify-between">
                <span>Matching Instruments</span>
                <span className="text-slate-500 font-normal">Equities</span>
              </div>
              <div className="max-h-64 overflow-y-auto divide-y divide-slate-800/60">
                {suggestions.map((item) => (
                  <button
                    key={item.symbol}
                    onClick={() => handleSelectSuggestion(item.symbol)}
                    className="w-full px-4 py-2.5 flex items-center justify-between hover:bg-slate-800/70 transition-colors text-left group"
                  >
                    <div>
                      <div className="font-semibold text-slate-200 group-hover:text-blue-400 transition-colors">
                        {item.symbol}
                      </div>
                      <div className="text-xs text-slate-400 truncate max-w-[220px]">
                        {item.companyName}
                      </div>
                    </div>
                    <div className="text-right">
                      <div className="font-mono font-medium text-slate-200">
                        ₹{item.currentPrice?.toFixed(2)}
                      </div>
                      <div className="text-[11px] text-slate-400">
                        {item.sector}
                      </div>
                    </div>
                  </button>
                ))}
              </div>
            </div>
          )}
        </div>

        {/* Right Action Icons & User Menu */}
        <div className="flex items-center space-x-3">
          {/* Notifications Dropdown */}
          <div ref={notifRef} className="relative">
            <button
              onClick={() => setShowNotifications(!showNotifications)}
              className="relative p-2 rounded-xl text-slate-300 hover:text-white hover:bg-slate-800/80 transition-colors focus:outline-none"
              title="Notifications"
            >
              <Bell className="w-5 h-5" />
              {unreadCount > 0 && (
                <span className="absolute top-1.5 right-1.5 w-4 h-4 bg-blue-500 text-white rounded-full text-[10px] font-bold flex items-center justify-center ring-2 ring-slate-900">
                  {unreadCount}
                </span>
              )}
            </button>

            {showNotifications && (
              <div className="absolute right-0 mt-2 w-80 bg-slate-900 border border-slate-700 rounded-xl shadow-2xl z-50 overflow-hidden">
                <div className="p-3 border-b border-slate-800 flex items-center justify-between bg-slate-800/40">
                  <h4 className="font-semibold text-sm text-slate-200">System Notifications</h4>
                  <span className="text-xs text-slate-400 font-mono">{notifications.length} alerts</span>
                </div>
                <div className="max-h-72 overflow-y-auto divide-y divide-slate-800/60">
                  {notifications.length === 0 ? (
                    <div className="p-6 text-center text-sm text-slate-400">No new notifications</div>
                  ) : (
                    notifications.map((n) => (
                      <div key={n.id} className="p-3 hover:bg-slate-800/40 transition-colors">
                        <div className="flex items-center space-x-2">
                          <CheckCircle className="w-3.5 h-3.5 text-blue-400" />
                          <span className="font-semibold text-xs text-slate-200">{n.title}</span>
                        </div>
                        <p className="text-xs text-slate-400 mt-1 pl-5">{n.message}</p>
                        <div className="flex items-center space-x-1 text-[10px] text-slate-500 mt-1 pl-5">
                          <Clock className="w-3 h-3" />
                          <span>{new Date(n.createdAt).toLocaleTimeString()}</span>
                        </div>
                      </div>
                    ))
                  )}
                </div>
              </div>
            )}
          </div>

          {/* User Profile Menu */}
          <div ref={profileRef} className="relative">
            <button
              onClick={() => setShowProfile(!showProfile)}
              className="flex items-center space-x-2 p-1.5 pr-2.5 rounded-xl hover:bg-slate-800/80 transition-colors focus:outline-none"
            >
              <div className="w-8 h-8 rounded-lg bg-gradient-to-tr from-blue-600 to-indigo-500 flex items-center justify-center font-bold text-white text-sm shadow">
                {user?.fullName?.charAt(0) || 'T'}
              </div>
              <div className="text-left hidden sm:block">
                <div className="text-xs font-semibold text-slate-200 leading-none">{user?.fullName || 'Trader'}</div>
                <div className="text-[10px] text-slate-400 capitalize">{user?.role || 'USER'}</div>
              </div>
              <ChevronDown className="w-3.5 h-3.5 text-slate-400" />
            </button>

            {showProfile && (
              <div className="absolute right-0 mt-2 w-56 bg-slate-900 border border-slate-700 rounded-xl shadow-2xl z-50 overflow-hidden py-1">
                <div className="px-4 py-2.5 border-b border-slate-800">
                  <p className="text-xs text-slate-400">Signed in as</p>
                  <p className="font-semibold text-sm text-slate-200 truncate">{user?.email}</p>
                </div>

                <Link
                  to="/portfolio"
                  onClick={() => setShowProfile(false)}
                  className="flex items-center px-4 py-2 text-sm text-slate-300 hover:bg-slate-800 hover:text-white"
                >
                  <UserIcon className="w-4 h-4 mr-2.5 text-slate-400" /> My Portfolio
                </Link>

                <Link
                  to="/funds"
                  onClick={() => setShowProfile(false)}
                  className="flex items-center px-4 py-2 text-sm text-slate-300 hover:bg-slate-800 hover:text-white"
                >
                  <Radio className="w-4 h-4 mr-2.5 text-slate-400" /> Funds & Wallet
                </Link>

                {isAdmin && (
                  <Link
                    to="/admin"
                    onClick={() => setShowProfile(false)}
                    className="flex items-center px-4 py-2 text-sm text-amber-300 hover:bg-slate-800 hover:text-amber-200"
                  >
                    <ShieldAlert className="w-4 h-4 mr-2.5 text-amber-400" /> Admin Console
                  </Link>
                )}

                <div className="border-t border-slate-800 my-1"></div>

                <button
                  onClick={() => {
                    setShowProfile(false);
                    logout();
                    navigate('/login');
                  }}
                  className="w-full flex items-center px-4 py-2 text-sm text-rose-400 hover:bg-rose-500/10 transition-colors"
                >
                  <LogOut className="w-4 h-4 mr-2.5 text-rose-400" /> Sign out
                </button>
              </div>
            )}
          </div>
        </div>
      </div>
    </header>
  );
};
