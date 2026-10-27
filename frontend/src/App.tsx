import React from 'react';
import { BrowserRouter, Routes, Route, Navigate, useLocation } from 'react-router-dom';
import { AuthProvider, useAuth } from './context/AuthContext';
import { MarketProvider } from './context/MarketContext';
import { Navbar } from './components/Navbar';
import { Sidebar } from './components/Sidebar';
import { MarketTicker } from './components/MarketTicker';
import { OrderModal } from './components/OrderModal';

// Pages
import { Dashboard } from './pages/Dashboard';
import { Markets } from './pages/Markets';
import { StockDetail } from './pages/StockDetail';
import { Watchlists } from './pages/Watchlists';
import { Orders } from './pages/Orders';
import { Portfolio } from './pages/Portfolio';
import { Funds } from './pages/Funds';
import { Transactions } from './pages/Transactions';
import { Admin } from './pages/Admin';
import { Login } from './pages/Login';
import { Register } from './pages/Register';
import { Settings } from './pages/Settings';

const ProtectedLayout: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const { user, loading } = useAuth();
  const location = useLocation();

  if (loading) {
    return (
      <div className="min-h-screen bg-slate-950 flex items-center justify-center">
        <div className="text-slate-400 font-mono text-xs flex items-center gap-2">
          <div className="w-4 h-4 border-2 border-blue-500 border-t-transparent rounded-full animate-spin" />
          <span>Initializing TradeX Environment...</span>
        </div>
      </div>
    );
  }

  if (!user) {
    return <Navigate to="/login" state={{ from: location }} replace />;
  }

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 flex flex-col font-sans">
      {/* Top Fixed Live Market Ticker */}
      <MarketTicker />

      {/* Main App Container */}
      <div className="flex flex-1 overflow-hidden">
        {/* Left Navigation Sidebar */}
        <Sidebar />

        {/* Content Area */}
        <div className="flex-1 flex flex-col min-w-0 overflow-y-auto">
          {/* Top Navbar */}
          <Navbar />

          {/* Page Body View */}
          <main className="flex-1 p-4 sm:p-6 max-w-7xl w-full mx-auto">
            {children}
          </main>
        </div>
      </div>

      {/* Global Order Execution Window (Modal) */}
      <OrderModal />
    </div>
  );
};

export const App: React.FC = () => {
  return (
    <BrowserRouter>
      <AuthProvider>
        <MarketProvider>
          <Routes>
            {/* Public Routes */}
            <Route path="/login" element={<Login />} />
            <Route path="/register" element={<Register />} />

            {/* Protected Routes */}
            <Route
              path="/"
              element={
                <ProtectedLayout>
                  <Dashboard />
                </ProtectedLayout>
              }
            />
            <Route
              path="/markets"
              element={
                <ProtectedLayout>
                  <Markets />
                </ProtectedLayout>
              }
            />
            <Route
              path="/stocks/:symbol"
              element={
                <ProtectedLayout>
                  <StockDetail />
                </ProtectedLayout>
              }
            />
            <Route
              path="/watchlists"
              element={
                <ProtectedLayout>
                  <Watchlists />
                </ProtectedLayout>
              }
            />
            <Route
              path="/orders"
              element={
                <ProtectedLayout>
                  <Orders />
                </ProtectedLayout>
              }
            />
            <Route
              path="/portfolio"
              element={
                <ProtectedLayout>
                  <Portfolio />
                </ProtectedLayout>
              }
            />
            <Route
              path="/holdings"
              element={
                <ProtectedLayout>
                  <Portfolio />
                </ProtectedLayout>
              }
            />
            <Route
              path="/positions"
              element={
                <ProtectedLayout>
                  <Portfolio />
                </ProtectedLayout>
              }
            />
            <Route
              path="/funds"
              element={
                <ProtectedLayout>
                  <Funds />
                </ProtectedLayout>
              }
            />
            <Route
              path="/transactions"
              element={
                <ProtectedLayout>
                  <Transactions />
                </ProtectedLayout>
              }
            />
            <Route
              path="/admin"
              element={
                <ProtectedLayout>
                  <Admin />
                </ProtectedLayout>
              }
            />
            <Route
              path="/settings"
              element={
                <ProtectedLayout>
                  <Settings />
                </ProtectedLayout>
              }
            />

            {/* Fallback Catch-all */}
            <Route path="*" element={<Navigate to="/" replace />} />
          </Routes>
        </MarketProvider>
      </AuthProvider>
    </BrowserRouter>
  );
};

export default App;
