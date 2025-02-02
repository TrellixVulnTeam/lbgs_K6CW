#include "rolemanager.h"
#include "src/globallogicserver.h"
#include "src/logicserver.h"
#include "role.h"

#include <logmodule/logdef.h>
#include <msgmodule/msgcommondef.h>

RoleManager::RoleManager()
{
}

RoleManager::~RoleManager()
{
	deleteInstance();
}

RoleManager* RoleManager::instanceObj = new(std::nothrow) RoleManager;
RoleManager* RoleManager::instance()
{
	return instanceObj;
}

void RoleManager::createRole(const CreateRoleInput& input)
{
	boost::shared_ptr<Role> newRole = boost::make_shared<Role>();
	newRole->setClientSeq(input.m_nClientSeq);
	newRole->setLoginParam(input.m_param);

	// insert to map
	CommonBoost::UniqueLock lock(m_mtxMap);
	m_mapIdToRole[input.m_param.m_strRoleId] = newRole;
	m_mapSeqToRole[input.m_nClientSeq] = newRole;
	lock.unlock();

	newRole->login();

}

void RoleManager::removeRole(ullong roleSeq, int errCode)
{
	if (m_mapIdToRole.empty())
	{
		LOG_ROLE.printLog("m_mapIdToRole empty, roleSeq[%lld]", roleSeq);
		return;
	}

	typedef std::map<std::string, boost::shared_ptr<Role>>::const_iterator MapRoleCIT;

	CommonBoost::UniqueLock lock(m_mtxMap);
	
	// delete from m_mapIdToRole
	boost::shared_ptr<Role> onceRole;
	ullong seq = 0;
	std::string id;
	for (MapRoleCIT cit = m_mapIdToRole.begin(); cit != m_mapIdToRole.end();)
	{
		onceRole = cit->second;

		if (!onceRole.get())
		{
			cit++;
		}


		seq = onceRole->getClientSeq();
		id = onceRole->getRoleId();

		if ((seq == roleSeq) &&
			(id.length() > 0))
		{
			onceRole->setLogoutErrorCode(errCode);
			onceRole->logout();
			m_mapIdToRole.erase(cit++);
			break;
		} else
		{
			cit++;
		}
		

		
	}

	// delete from m_mapSeqToRole
	if (onceRole.get())
	{
		m_mapSeqToRole.erase(onceRole->getClientSeq());
	}
}

bool RoleManager::isRoleExists(const std::string& roleId)
{
	std::map<std::string, boost::shared_ptr<Role>>::const_iterator cit =
		m_mapIdToRole.find(roleId);

	return (cit != m_mapIdToRole.end());
}

bool RoleManager::isRoleExists(ullong roleSeq)
{
	std::map<ullong, boost::shared_ptr<Role>>::const_iterator cit =
		m_mapSeqToRole.find(roleSeq);

	return (cit != m_mapSeqToRole.end());
}

boost::shared_ptr<Role> RoleManager::findRoleByClientSeq(ullong clientSeq)
{
	std::map<ullong, boost::shared_ptr<Role>>::const_iterator cit =
		m_mapSeqToRole.find(clientSeq);
	if (cit != m_mapSeqToRole.cend())
	{
		return cit->second;
	}
	return NULL;
}

void RoleManager::deleteInstance()
{
	if (instanceObj)
	{
		delete instanceObj;
		instanceObj = NULL;
	}
}
